cmake_minimum_required(VERSION 2.6)
project(fusion)

add_subdirectory(pidwatch)
add_subdirectory(fautes)

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
