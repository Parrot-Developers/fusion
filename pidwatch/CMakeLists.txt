cmake_minimum_required(VERSION 2.8)
project(pidwatch)
include_directories(include)

option(PIDWATCH_FAUTES_SUPPORT "enable automated tests" True)

install(FILES include/pidwatch.h DESTINATION include)
set(LIBPIDWATCH_SOURCES
    src/pidwatch.c
    include/pidwatch.h)
set(LIBPIDWATCH_LINK_LIBRARIES)
if (${PIDWATCH_FAUTES_SUPPORT})
    list(APPEND LIBPIDWATCH_SOURCES
        tests/pw_fautes.c
        tests/pw_fautes.h
        tests/pw_tests.c)
    find_library(CUNIT_LIB cunit)
    set(LIBPIDWATCH_LINK_LIBRARIES
        libfautes
        ${CUNIT_LIB})
endif(${PIDWATCH_FAUTES_SUPPORT})
add_library(pidwatch SHARED ${LIBPIDWATCH_SOURCES})
target_link_libraries(pidwatch ${LIBPIDWATCH_LINK_LIBRARIES})
set_target_properties(pidwatch PROPERTIES LINK_FLAGS "-Wl,-e,libpidwatch_tests")
install(TARGETS pidwatch DESTINATION lib)

add_executable(pidwait example/pidwait.c)
target_link_libraries(pidwait pidwatch)
install(TARGETS pidwait DESTINATION bin)
