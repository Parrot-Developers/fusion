LOCAL_PATH := $(call my-dir)

###############################################################################
# libfautes
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libfautes
LOCAL_DESCRIPTION := Library for adding fautes support to shared libraries
LOCAL_CATEGORY_PATH := devel/unit_tests

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_EXPORT_LDLIBS := -lcunit

LOCAL_SRC_FILES := \
	$(call all-c-files-under,lib) \

LOCAL_LIBRARIES := libcunit

include $(BUILD_LIBRARY)

###############################################################################
# fautes
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := fautes
LOCAL_DESCRIPTION := Utility to run embedded unit tests using libfautes / cunit
LOCAL_CATEGORY_PATH := devel/unit_tests

LOCAL_SRC_FILES := \
	$(call all-c-files-under,src) \

LOCAL_LDLIBS := -ldl

LOCAL_LIBRARIES := libfautes libcunit

include $(BUILD_EXECUTABLE)


