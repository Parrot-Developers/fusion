cmake_minimum_required(VERSION 2.6)
project(pidwatch)
include_directories(include)

add_library(pidwatch
    SHARED
    src/pidwatch.c
    include/pidwatch.h)
install(FILES include/pidwatch.h DESTINATION include)
install(TARGETS pidwatch DESTINATION lib)

add_executable(pidwait example/pidwait.c)
target_link_libraries(pidwait pidwatch)
install(TARGETS pidwait DESTINATION bin)
