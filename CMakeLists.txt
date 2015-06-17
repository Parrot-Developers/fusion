cmake_minimum_required(VERSION 2.8)
project(fusion)

# for building executable shared libraries, able to execute autonomously their
# automatic tests, we need to set the .inter program header. For now, I couldn't
# find a better way than to build and link an executable and read it's .interp
# section
execute_process(COMMAND sh -c "echo 'int main;' | \
        ${CMAKE_C_COMPILER} -xc -; readelf -l a.out | \
        grep 'interpreter:' | \
        sed 's/.*: \\(.*\\)\\]/\\1/g'"
    OUTPUT_VARIABLE FUSION_INTERPRETER)
string(STRIP ${FUSION_INTERPRETER} FUSION_INTERPRETER)
message("Detected interpreter: " ${FUSION_INTERPRETER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -DFUSION_INTERPRETER=\\\"${FUSION_INTERPRETER}\\\"")

include_directories(fautes/include)
include_directories(libutils/include)
include_directories(pidwatch/include)
include_directories(librs/include)

add_subdirectory(fautes)
add_subdirectory(pidwatch)
add_subdirectory(libutils)
add_subdirectory(librs)
add_subdirectory(libioutils)

# build a CPack driven installer package
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENCE")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "2")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER
    "Nicolas Carrier - carrier.nicolas0@gmail.com")
include (CPack)
