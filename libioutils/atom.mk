LOCAL_PATH := $(call my-dir)

###############################################################################
# libioutils
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libioutils
LOCAL_DESCRIPTION := Utils for event loop based asynchronous I/O
LOCAL_CATEGORY_PATH := libs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(call all-c-files-under,src)

LOCAL_LIBRARIES := librs libpidwatch

ifdef TARGET_TEST
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES += libfautes
endif

include $(BUILD_LIBRARY)

###############################################################################
# tst-libioutils
###############################################################################

ifdef TARGET_TEST
include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := tests

TESTED_MODULE := libioutils

LOCAL_MODULE := tst-$(TESTED_MODULE)

LOCAL_REQUIRED_MODULES := fautes $(TESTED_MODULE)

LOCAL_COPY_FILES := \
	tests/tst-01.sh:tests/bin/$(TESTED_MODULE)/ \
	tests/email_notification:tests/bin/$(TESTED_MODULE)/

include $(BUILD_CUSTOM)
endif # TARGET_TEST