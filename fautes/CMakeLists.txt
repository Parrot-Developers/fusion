cmake_minimum_required(VERSION 2.6)
project(fautes)
include_directories(include)

# the lib's target should be named fautes, but because it clashes with the
# executable's name, I rename it to libfaute, which forces me to override it's
# output_name with set_target_properties()
add_library(libfautes
    SHARED
    lib/fautes_utils.c
    include/fautes.h
    include/fautes_utils.h)
set_target_properties(libfautes PROPERTIES OUTPUT_NAME  fautes)
install(TARGETS libfautes DESTINATION lib)
install(FILES include/fautes.h DESTINATION include)
install(FILES include/fautes_utils.h DESTINATION include)

add_executable(fautes src/fautes.c)
find_library(CUNIT_LIB cunit)
target_link_libraries(fautes
    libfautes
    ${CUNIT_LIB}
    ${CMAKE_DL_LIBS})
install(TARGETS fautes DESTINATION bin)
