cmake_minimum_required(VERSION 2.8)
project(fusion)

include_directories(fautes/include)
include_directories(libutils/include)

add_subdirectory(fautes)
add_subdirectory(pidwatch)
add_subdirectory(libutils)
add_subdirectory(librs)

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
