#!/bin/sh

set -e

module_name=libioutils

# Note src_pid test suite is disabled since it requires root priviledges
FAUTES_SUITE_ACTIVE_STATE_src_pid_suite=0 \
		PROCESS_TEST_SCRIPT=./packages/fusion/libioutils/tests/test.process \
		fautes xml ${module_name}.so

if [ -n "${CUNIT_OUT_NAME}" ]; then
	mv ${module_name}-Results.xml ${CUNIT_OUT_NAME}-Results.xml
	mv ${module_name}-Listing.xml ${CUNIT_OUT_NAME}-Listing.xml
fi
