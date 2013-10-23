#!/bin/sh

set -e

module_name=libpidwatch

# ${module_name}.so for a library, /full/path/${module_name} for an executable
path=${module_name}.so
result_dir=${TEST_RESULT_DIR:-/tmp/${module_name}/}

mkdir -p ${result_dir}
cd ${result_dir}
fautes xml ${path}
mv ${module_name}-Listing.xml CUnit-${module_name}-Listing.xml
mv ${module_name}-Results.xml CUnit-${module_name}-Results.xml
