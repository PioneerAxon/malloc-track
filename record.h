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

#ifndef __MALLOC_TRACK_RECORD_H__
#define __MALLOC_TRACK_RECORD_H__

#include <inttypes.h>
#include <sys/types.h>

#include "malloc_track.h"

enum
{
	kMallocRecord,
	kFreeRecord
};

typedef struct malloc_track_record
{
	uint64_t timestamp;
	int32_t type;
	void *address;
	size_t size;
	pid_t thread_id;
	uint32_t stack_entries;
	void *frames[0];
} malloc_track_record_t;

void record_create_malloc(void *p, size_t size);
void record_create_free(void *p);

uint32_t malloc_track_record_t_size(malloc_track_record_t *record);

#endif
