LOCAL_PATH := $(call my-dir)

###############################################################################
# libpidwatch
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libpidwatch
LOCAL_DESCRIPTION := Library for monitoring the death of processes with an fd
LOCAL_CATEGORY_PATH := libs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	$(call all-c-files-under,src) \

ifdef TARGET_TEST
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES += libfautes
endif

include $(BUILD_SHARED_LIBRARY)

###############################################################################
# pidwait
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := pidwait
LOCAL_DESCRIPTION := Example libpidwatch client for monitoring processes death
LOCAL_CATEGORY_PATH := devel

LOCAL_SRC_FILES := \
	$(call all-c-files-under,example) \

LOCAL_LIBRARIES := libpidwatch

include $(BUILD_EXECUTABLE)

###############################################################################
# tst-libpidwatch
###############################################################################

ifdef TARGET_TEST
include $(CLEAR_VARS)

LOCAL_CATEGORY_PATH := tests

TESTED_MODULE := libpidwatch

LOCAL_MODULE := tst-$(TESTED_MODULE)

LOCAL_REQUIRED_MODULES := fautes $(TESTED_MODULE)

LOCAL_COPY_FILES := \
	tests/tst-01.sh:tests/bin/$(TESTED_MODULE)/ \
	tests/email_notification:tests/bin/$(TESTED_MODULE)/

include $(BUILD_CUSTOM)
endif # TARGET_TEST