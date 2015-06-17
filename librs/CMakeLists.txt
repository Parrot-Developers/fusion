cmake_minimum_required(VERSION 2.8)
project(rs)
include_directories(include)

option(RS_FAUTES_SUPPORT "enable automated tests" True)

file(GLOB RS_HEADERS include/*.h)
install(FILES ${RS_HEADERS} DESTINATION include)
file(GLOB RS_SOURCES src/*.c)
set(RS_SOURCES ${RS_SOURCES} ${RS_HEADERS})
set(RS_LINK_LIBRARIES utils)
if (${RS_FAUTES_SUPPORT})
    file(GLOB RS_FAUTES_SOURCES tests/*.[ch])
    list(APPEND RS_SOURCES ${RS_FAUTES_SOURCES})
    find_library(CUNIT_LIB cunit)
    list(APPEND RS_LINK_LIBRARIES
        libfautes
        ${CUNIT_LIB})
endif(${RS_FAUTES_SUPPORT})
add_library(rs SHARED ${RS_SOURCES})
target_link_libraries(rs ${RS_LINK_LIBRARIES})
set_target_properties(rs PROPERTIES LINK_FLAGS "-Wl,-e,librs_tests")
install(TARGETS rs DESTINATION lib)
