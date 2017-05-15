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

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include "malloc_track.h"
#include "ring_buffer.h"
#include "record.h"

#define init_buffer_max_size 4096

static void* (*real_malloc)(size_t)=NULL;
static void (*real_free)(void*)=NULL;
static void* (*real_realloc)(void*, size_t)=NULL;
static void* (*real_calloc)(size_t, size_t)=NULL;
static int32_t malloc_track_destroying_ = 0;
static int32_t malloc_track_initializing_ = 0;

uint8_t init_buffer_[init_buffer_max_size];
uint32_t init_buffer_offset_ = 0;

static void mt_init(void)
{
	malloc_track_initializing_ = 1;
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	real_free = dlsym(RTLD_NEXT, "free");
	real_calloc = dlsym(RTLD_NEXT, "calloc");
	real_realloc = dlsym(RTLD_NEXT, "realloc");
	assert(real_free && real_malloc && real_calloc && real_free);
	ring_buffer_new();
	malloc_track_initializing_ = 0;
}

static void __attribute__((destructor)) mt_destroy(void)
{
	malloc_track_destroying_ = 1;
	ring_buffer_delete();
}

void *malloc(size_t size)
{
	if(!real_malloc)
	{
		mt_init();
	}

	void *p = NULL;
	p = real_malloc(size);
	if (!malloc_track_destroying_)
	{
		DEBUG("malloc(%d): %p\n", size, p);
		record_create_malloc(p, size);
	}
	return p;
}

void free(void* p)
{
	if (!real_free)
	{
		mt_init();
	}

	if (p >= (void*)init_buffer_ && p < (void*)(init_buffer_ + init_buffer_max_size))
	{
		return;
	}

	if (!malloc_track_destroying_)
	{
		DEBUG("free: %p\n", p);
		record_create_free(p);
	}
	real_free(p);
}

void *calloc(size_t nmemb, size_t size)
{
	if (!real_calloc)
	{
		if (malloc_track_initializing_)
		{
			assert(init_buffer_offset_ + (nmemb * size) < init_buffer_max_size);
			memset(init_buffer_ + init_buffer_offset_, 0, nmemb * size);
			void *p = init_buffer_ + init_buffer_offset_;
			init_buffer_offset_ += nmemb * size;
			DEBUG("Returning %lld bytes at address %p from pre-allocated memory to calloc().\n", nmemb * size, p);
			return p;
		}
		mt_init();
	}

	void *p = NULL;
	p = real_calloc(nmemb, size);
	if (!malloc_track_destroying_)
	{
		DEBUG("calloc(%d, %d): %p\n", nmemb, size, p);
		record_create_calloc(p, nmemb, size);
	}
	return p;
}

void *realloc(void* ptr, size_t size)
{
	if (!real_realloc)
	{
		mt_init();
	}

	void *p = NULL;
	p = real_realloc(ptr, size);
	if (!malloc_track_destroying_)
	{
		DEBUG("realloc(%p, %d): %p\n", ptr, size, p);
		record_create_realloc(p, ptr, size);
	}
	return p;
}

void *mt_malloc(size_t size)
{
	DEBUG_ASSERT(real_malloc);
	return real_malloc(size);
}

void mt_free(void *p)
{
	DEBUG_ASSERT(real_free);
	real_free(p);
}
