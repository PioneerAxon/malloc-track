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

#include "../record_struct.h"

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

	int64_t total_records = 0;
	int64_t malloc_count = 0;
	int64_t calloc_count = 0;
	int64_t realloc_count = 0;
	int64_t free_count = 0;

	malloc_track_record_t record;
	while (read(fd, &record, sizeof(malloc_track_record_t)) == sizeof(malloc_track_record_t))
	{
		printf("%llu %d %u %p %llu %llu", record.timestamp, record.type, record.stack_entries, record.address, record.size, record.thread_id);

		uint32_t frames = record.stack_entries;
		uint64_t stack_frame;
		while (frames--)
		{
			if (read(fd, &stack_frame, sizeof(uint64_t)) == sizeof(uint64_t))
			{
				printf(" %p", stack_frame);
			}
			else
			{
				premature_eof();
			}
		}

		printf("\n");
		total_records++;
		if (record.type == kMallocRecord)
			malloc_count++;
		else if (record.type == kFreeRecord && record.address != 0)
			free_count++;
		else if (record.type == kCallocRecord)
			calloc_count++;
		else if (record.type == kReallocRecord)
			realloc_count++;
	}
	close(fd);
	fprintf(stderr, "Total records : %lld\n      malloc  : %lld\n      calloc  : %lld\n      realloc : %lld\n      free    : %lld\n", total_records, malloc_count, calloc_count, realloc_count, free_count);
	return 0;
}
