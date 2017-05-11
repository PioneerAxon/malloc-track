add_library (malloc_track SHARED malloc_track.c)

target_link_libraries(malloc_track dl)
