cmake_minimum_required(VERSION 3.0)

add_library (malloc_track SHARED malloc_track.c ring_buffer.c record.c)

target_link_libraries(malloc_track dl)

add_subdirectory(analysis)
add_subdirectory(tests)
