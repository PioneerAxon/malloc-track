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
#include <execinfo.h>

#include "malloc_track.h"
#include "record.h"
#include "ring_buffer.h"

static const uint32_t record_max_stack_depth = 10;
static int32_t backtrace_initiated = 0;
static int32_t backtrace_initiating = 0;

uint64_t record_get_time_usecs()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return (now.tv_sec * 1000000ull) + (now.tv_nsec / 1000ull);
}

static void fill_backtrace(malloc_track_record_t *record)
{
	if (backtrace_initiating)
	{
		record->stack_entries = 0;
	}
	else
	{
		if (!backtrace_initiated)
		{
			backtrace_initiating = 1;
		}
		record->stack_entries = backtrace((void**)record->frames, record_max_stack_depth);
		if (!backtrace_initiated)
		{
			backtrace_initiated = 1;
			backtrace_initiating = 0;
		}
	}
}

void record_create_malloc(void *p, size_t size)
{
	malloc_track_record_t *record = (malloc_track_record_t*)mt_malloc(sizeof(malloc_track_record_t) + sizeof(uint64_t) * record_max_stack_depth);
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kMallocRecord;
	record->address = (uint64_t)p;
	record->size = size;
	record->thread_id = syscall(__NR_gettid);
	fill_backtrace(record);
	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

void record_create_free(void *p)
{
	malloc_track_record_t *record = (malloc_track_record_t*)mt_malloc(sizeof(malloc_track_record_t) + sizeof(uint64_t) * record_max_stack_depth);
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kFreeRecord;
	record->address = (uint64_t)p;
	record->size = 0;
	record->thread_id = syscall(__NR_gettid);
	fill_backtrace(record);
	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

void record_create_calloc(void *p, size_t nmemb, size_t size)
{
	malloc_track_record_t *record = (malloc_track_record_t*)mt_malloc(sizeof(malloc_track_record_t) + sizeof(uint64_t) * record_max_stack_depth);
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kCallocRecord;
	record->address = (uint64_t)p;
	record->size = size;
	record->nmemb = nmemb;
	record->thread_id = syscall(__NR_gettid);
	fill_backtrace(record);
	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

void record_create_realloc(void *p, void *ptr, size_t size)
{
	malloc_track_record_t *record = (malloc_track_record_t*)mt_malloc(sizeof(malloc_track_record_t) + sizeof(uint64_t) * record_max_stack_depth);
	assert(record);
	record->timestamp = record_get_time_usecs();
	record->type = kReallocRecord;
	record->address = (uint64_t)p;
	record->size = size;
	record->old_address = (uint64_t)ptr;
	record->thread_id = syscall(__NR_gettid);
	fill_backtrace(record);
	ring_buffer_insert_lock_free(record);
	mt_free(record);
}

uint32_t malloc_track_record_t_size(malloc_track_record_t *record)
{
	DEBUG_ASSERT(record);
	return sizeof(malloc_track_record_t) + (record->stack_entries * sizeof(uint64_t));
}
