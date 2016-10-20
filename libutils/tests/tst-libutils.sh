#!/bin/sh

set -e

module_name=libutils
fautes xml ${module_name}.so

if [ -n "${CUNIT_OUT_NAME}" ]; then
	mv ${module_name}-Results.xml ${CUNIT_OUT_NAME}-Results.xml
	mv ${module_name}-Listing.xml ${CUNIT_OUT_NAME}-Listing.xml
fi
