LOCAL_PATH := $(call my-dir)

###############################################################################
# librs
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := librs
LOCAL_DESCRIPTION := Library implementing some base sets in a robust way
LOCAL_CATEGORY_PATH := libs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(call all-c-files-under,src)

ifdef TARGET_TEST
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES := libfautes
endif

include $(BUILD_SHARED_LIBRARY)

###############################################################################
# tst-librs
###############################################################################

ifdef TARGET_TEST
include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := tests

LOCAL_MODULE := tst-librs

LOCAL_REQUIRED_MODULES := fautes librs

LOCAL_COPY_FILES := \
	tests/tst-01.sh:tests/bin/librs/ \
	tests/email_notification:tests/bin/librs/

include $(BUILD_CUSTOM)
endif # TARGET_TEST