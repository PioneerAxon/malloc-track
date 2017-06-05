add_executable(single-thread-stress single-thread-stress.c)

add_executable(multi-thread-stress multi-thread-stress.c)
target_link_libraries(multi-thread-stress pthread)
