LOCAL_PATH := $(call my-dir)

###############################################################################
# libutils
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libutils
LOCAL_DESCRIPTION := Library implementing some base utilities for not \
	reinventing the wheel constantly...
LOCAL_CATEGORY_PATH := libs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(call all-c-files-under,src)

ifdef TARGET_TEST
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES := libfautes
endif # TARGET_TEST

include $(BUILD_LIBRARY)

###############################################################################
# tst-libutils
###############################################################################

ifdef TARGET_TEST
include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := tests

TESTED_MODULE := libutils

LOCAL_MODULE := tst-$(TESTED_MODULE)

LOCAL_REQUIRED_MODULES := fautes $(TESTED_MODULE)

LOCAL_COPY_FILES := \
	tests/tst-01.sh:tests/bin/$(TESTED_MODULE)/ \
	tests/email_notification:tests/bin/$(TESTED_MODULE)/

include $(BUILD_CUSTOM)
endif # TARGET_TEST