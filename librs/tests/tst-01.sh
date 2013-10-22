#!/bin/sh

result_dir=/tests/results/librs

mkdir -p ${result_dir}
cd ${result_dir}
fautes xml librs.so
mv librs-Listing.xml CUnit-librs-Listing.xml
mv librs-Results.xml CUnit-librs-Results.xml
