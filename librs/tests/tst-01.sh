#!/bin/sh

result_dir=${TEST_RESULT_DIR:-/tmp/librs/}

mkdir -p ${result_dir}
cd ${result_dir}
fautes xml librs.so
mv librs-Listing.xml CUnit-librs-Listing.xml
mv librs-Results.xml CUnit-librs-Results.xml
