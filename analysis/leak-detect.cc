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

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <map>

#include "../record_struct.h"

using namespace std;

void print_usage(char *program_name)
{
	printf("Usage : %s <file-path>\n", program_name);
}

void premature_eof()
{
	fprintf(stderr, "Premature eof while trying to read the record\n");
	exit(0);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		print_usage(argv[0]);
		return 0;
	}
	char *file_name = argv[1];
	int fd = open(file_name, O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "Error opening file '%s' : ", file_name);
		perror(NULL);
		exit(1);
	}

	map<uint64_t, malloc_track_record_t*> live_memory;

	malloc_track_record_t record;
	while (read(fd, &record, sizeof(malloc_track_record_t)) == sizeof(malloc_track_record_t))
	{
		malloc_track_record_t *cur = (malloc_track_record_t*) malloc(sizeof(malloc_track_record_t) + record.stack_entries * sizeof(uint64_t));
		memcpy(cur, &record, sizeof(malloc_track_record_t));

		for (uint32_t i = 0; i < record.stack_entries; i++)
		{
			if (read(fd, &cur->frames[i], sizeof(uint64_t)) != sizeof(uint64_t))
			{
				premature_eof();
			}
		}
		if (record.type == kMallocRecord)
		{
			assert(!live_memory[record.address]);
			live_memory[record.address] = cur;
		}
		else if (record.type == kFreeRecord && record.address != 0)
		{
			assert(live_memory[record.address]);
			free(live_memory[record.address]);
			live_memory.erase(record.address);
		}
	}
	close(fd);

	if (live_memory.size())
	{
		printf("Memory leaks detected : %lu\n", live_memory.size());
		for (map<uint64_t, malloc_track_record_t*>::iterator iter = live_memory.begin(); iter != live_memory.end(); ++iter)
		{
			printf("%llu bytes allocated from thread %llu at time %llu with address %p. Callstack : ", iter->second->size, iter->second->thread_id, iter->second->timestamp, iter->second->address);
			for (uint32_t i = 0; i < iter->second->stack_entries; i++)
			{
				printf (" %p", iter->second->frames[i]);
			}
			printf ("\n");
		}
	}

	return 0;
}
