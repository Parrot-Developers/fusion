cmake_minimum_required(VERSION 2.8)
project(utils)
include_directories(include)

option(UTILS_FAUTES_SUPPORT "enable automated tests" True)

file(GLOB UTILS_HEADERS include/*.h)
install(FILES ${UTILS_HEADERS} DESTINATION include)
file(GLOB UTILS_SOURCES src/*.c)
set(UTILS_SOURCES ${UTILS_SOURCES} ${UTILS_HEADERS})
set(UTILS_LINK_LIBRARIES)
if (${UTILS_FAUTES_SUPPORT})
    file(GLOB UTILS_FAUTES_SOURCES tests/*.[ch])
    list(APPEND UTILS_SOURCES ${UTILS_FAUTES_SOURCES})
    find_library(CUNIT_LIB cunit)
    set(UTILS_LINK_LIBRARIES
        libfautes
        ${CUNIT_LIB})
endif(${UTILS_FAUTES_SUPPORT})
add_library(utils SHARED ${UTILS_SOURCES})
target_link_libraries(utils ${UTILS_LINK_LIBRARIES})
set_target_properties(utils PROPERTIES LINK_FLAGS "-Wl,-e,libutils_tests")
install(TARGETS utils DESTINATION lib)
