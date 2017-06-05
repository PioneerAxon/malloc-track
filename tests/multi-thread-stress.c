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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static const int num_thread = 10;

void *malloc_test(void *id)
{
	void *mem;
	for (int i = 0; i < 1000000; i++)
	{
		mem = malloc(i);
		printf("%ld %d %p\n", (int64_t)id, i, mem);
		free(mem);
	}
	return NULL;
}

int main()
{
	pthread_t threads[num_thread];
	for (int64_t i = 0; i < num_thread; i++)
	{
		pthread_create(&threads[i], NULL, malloc_test, (void*)i);
	}
	for (int64_t i = 0; i < num_thread; i++)
	{
		pthread_join(threads[i], NULL);
	}
	return 0;
}
