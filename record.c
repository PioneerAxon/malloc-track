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

#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

#include "malloc_track.h"
#include "record.h"
#include "ring_buffer.h"

uint64_t record_get_time_usecs()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return (now.tv_sec * 1000000ull) + (now.tv_nsec / 1000ull);
}

void record_create_malloc(void *p, size_t size)
{
	malloc_track_record_t *record = mt_malloc(sizeof(malloc_track_record_t));
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kMallocRecord;
	record->address = p;
	record->size = size;
	record->thread_id = syscall(__NR_gettid);
	record->stack_entries = 0;

	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

void record_create_free(void *p)
{
	malloc_track_record_t *record = mt_malloc(sizeof(malloc_track_record_t));
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kFreeRecord;
	record->address = p;
	record->size = 0;
	record->thread_id = syscall(__NR_gettid);
	record->stack_entries = 0;

	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

uint32_t malloc_track_record_t_size(malloc_track_record_t *record)
{
	DEBUG_ASSERT(record);
	return sizeof(malloc_track_record_t) + (record->stack_entries * sizeof(void*));
}
