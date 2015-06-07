cmake_minimum_required (VERSION 2.6)
project (pidwatch)

include_directories(include)

add_executable(pidwait example/pidwait.c)

add_library(pidwatch
    SHARED
    src/pidwatch.c
    include/pidwatch.h)

target_link_libraries(pidwait pidwatch)

install(TARGETS pidwatch DESTINATION lib)
install(FILES include/pidwatch.h DESTINATION include)
install(TARGETS pidwait DESTINATION bin)
