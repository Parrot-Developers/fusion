#!/bin/sh

set -e

module_name=libpidwatch

# ${module_name}.so for a library, /full/path/${module_name} for an executable
path=${module_name}.so
result_dir=${TEST_RESULT_DIR:-/tmp/${module_name}/}

mkdir -p ${result_dir}
cd ${result_dir}
fautes xml ${path}
mv ${module_name}-Listing.xml CUnit-tst-01-Listing.xml
mv ${module_name}-Results.xml CUnit-tst-01-Results.xml
