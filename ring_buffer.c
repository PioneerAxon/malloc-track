/*
 * Copyright 2017 Arth Patel (PioneerAxon) <arth.svnit@gmail.com>
 *
 * This file is part of MallocTrack.
 *
 * MallocTrack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MallocTrack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "ring_buffer.h"

static const int32_t ring_buffer_size_bytes_exp = 24; //16 MiB
static const int32_t ring_buffer_max_active_bytes = 1 << 20; // 1 MiB
static const int32_t ring_buffer_flush_batch_size = 1 << 12; // 4KiB

static void* ring_buffer_ = NULL;
static uint64_t ring_buffer_size_ = 0;
static uint64_t ring_buffer_offset_mask_ = 0;

static uint64_t ring_buffer_write_offset_ = 0;
static uint64_t ring_buffer_flush_offset_ = 0;
static uint64_t ring_buffer_active_bytes_ = 0;
static short ring_buffer_flush_active_ = 0;

static char ring_buffer_flush_file_name[] = "/tmp/binarydump";
static int64_t ring_buffer_flush_fd_;

static inline uint64_t max_u64(uint64_t a, uint64_t b)
{
	return a > b ? a : b;
}

static inline uint64_t min_u64(uint64_t a, uint64_t b)
{
	return a < b ? a : b;
}

void ring_buffer_new()
{
	assert(ring_buffer_ == NULL);
	ring_buffer_size_ = 1 << ring_buffer_size_bytes_exp;
	ring_buffer_offset_mask_ = ring_buffer_size_ - 1;
	ring_buffer_ = mt_malloc (ring_buffer_size_);
	assert(ring_buffer_);
	DEBUG("Allocated ring buffer of size %llu\n", ring_buffer_size_);
	DEBUG("Opening file %s\n", ring_buffer_flush_file_name);
	ring_buffer_flush_fd_ = open(ring_buffer_flush_file_name, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	DEBUG_ASSERT(ring_buffer_flush_fd_ != -1);
	if (ring_buffer_flush_fd_ == -1)
	{
		exit(1);
	}
}

void ring_buffer_delete()
{
	DEBUG("Destroying ring buffer. Flushing all records\n");
	ring_buffer_flush_all();
	DEBUG("Closing file %s\n", ring_buffer_flush_file_name);
	close(ring_buffer_flush_fd_);
	assert(ring_buffer_);
	mt_free(ring_buffer_);
	ring_buffer_size_ = 0;
	ring_buffer_ = NULL;
}

void ring_buffer_insert_lock_free(malloc_track_record_t *record)
{
	uint32_t size = malloc_track_record_t_size(record);
	uint32_t bytes_left = size;
	uint64_t write_offset = __sync_fetch_and_add(&ring_buffer_write_offset_, size);
	__sync_fetch_and_and(&ring_buffer_write_offset_, ring_buffer_offset_mask_);

	DEBUG_ASSERT(size < ring_buffer_size_);

	while (bytes_left)
	{
		uint32_t bytes_to_write = min_u64(ring_buffer_size_ - write_offset, bytes_left);
		memcpy(ring_buffer_ + write_offset, record, bytes_to_write);
		bytes_left -= bytes_to_write;
	}
	uint64_t active_bytes = __sync_add_and_fetch(&ring_buffer_active_bytes_, size);
	DEBUG("Active bytes in ring buffer : %llu\n", active_bytes);
	if (active_bytes > ring_buffer_max_active_bytes)
	{
		ring_buffer_maybe_dump_bytes(active_bytes);
	}
}

void ring_buffer_maybe_dump_bytes(uint64_t active_bytes)
{
	if (!__sync_bool_compare_and_swap(&ring_buffer_flush_active_, 0, 1))
	{
		return;
	}
	while (active_bytes > ring_buffer_max_active_bytes)
	{
		uint64_t bytes_written = write(ring_buffer_flush_fd_, ring_buffer_ + ring_buffer_flush_offset_, min_u64(ring_buffer_size_ - ring_buffer_flush_offset_, ring_buffer_flush_batch_size));
		ring_buffer_flush_offset_ += bytes_written;
		ring_buffer_flush_offset_ &= ring_buffer_offset_mask_;
		DEBUG("Flushed %llu bytes\n", bytes_written);

		active_bytes = __sync_sub_and_fetch(&ring_buffer_active_bytes_, bytes_written);
	}
	ring_buffer_flush_active_ = 0;
}

void ring_buffer_flush_all()
{
	while(!__sync_bool_compare_and_swap(&ring_buffer_flush_active_, 0, 1));
	uint64_t active_bytes = __sync_fetch_and_add(&ring_buffer_active_bytes_, 0);
	while (active_bytes > 0)
	{
		uint64_t bytes_written = write(ring_buffer_flush_fd_, ring_buffer_ + ring_buffer_flush_offset_, min_u64(min_u64(ring_buffer_size_ - ring_buffer_flush_offset_, ring_buffer_flush_batch_size), active_bytes));
		ring_buffer_flush_offset_ += bytes_written;
		ring_buffer_flush_offset_ &= ring_buffer_offset_mask_;
		active_bytes = __sync_sub_and_fetch(&ring_buffer_active_bytes_, bytes_written);
		DEBUG("Flushed %llu bytes. %llu bytes remaining.\n", bytes_written, active_bytes);
	}
	ring_buffer_flush_active_ = 0;
}
