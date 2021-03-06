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

#ifndef __MALLOC_TRACK_RING_BUFFER_H__
#define __MALLOC_TRACK_RING_BUFFER_H__

#include "record.h"

void ring_buffer_new();

void ring_buffer_delete();

void ring_buffer_insert_lock_free(malloc_track_record_t *record);

void ring_buffer_maybe_dump_bytes(uint64_t active_bytes);

void ring_buffer_flush_all();

#endif
