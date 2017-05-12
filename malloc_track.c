/*
 *
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
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <assert.h>

static void* (*real_malloc)(size_t)=NULL;
static void (*real_free)(void*)=NULL;

static void mtrace_init(void)
{
	real_malloc = dlsym(RTLD_NEXT, "malloc");
	real_free = dlsym(RTLD_NEXT, "free");
	assert(real_free && real_malloc);
}

void *malloc(size_t size)
{
	if(!real_malloc)
	{
		mtrace_init();
	}

	void *p = NULL;
	p = real_malloc(size);
	return p;
}

void free(void* p)
{
	if (!real_free)
	{
		mtrace_init();
	}

	real_free(p);
}
