cmake_minimum_required(VERSION 2.8)
project(ioutils)
include_directories(include)

option(IOUTILS_FAUTES_SUPPORT "enable automated tests" True)

file(GLOB IOUTILS_HEADERS include/*.h)
install(FILES ${IOUTILS_HEADERS} DESTINATION include)
file(GLOB IOUTILS_SOURCES src/*.c)
find_package(Threads)
set(IOUTILS_SOURCES ${IOUTILS_SOURCES} ${IOUTILS_HEADERS})
set(IOUTILS_LINK_LIBRARIES utils rs pidwatch ${CMAKE_THREAD_LIBS_INIT})
if (${IOUTILS_FAUTES_SUPPORT})
    file(GLOB IOUTILS_FAUTES_SOURCES tests/*.[ch])
    list(APPEND IOUTILS_SOURCES ${IOUTILS_FAUTES_SOURCES})
    find_library(CUNIT_LIB cunit)
    list(APPEND IOUTILS_LINK_LIBRARIES
        libfautes
        ${CUNIT_LIB})
endif(${IOUTILS_FAUTES_SUPPORT})
add_library(ioutils SHARED ${IOUTILS_SOURCES})
target_link_libraries(ioutils ${IOUTILS_LINK_LIBRARIES})
set_target_properties(ioutils PROPERTIES LINK_FLAGS "-Wl,-e,libioutils_tests")
install(TARGETS ioutils DESTINATION lib)
