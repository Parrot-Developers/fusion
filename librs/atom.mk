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

ifdef LIBRS_FAUTES_SUPPORT
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES := libfautes
endif

include $(BUILD_SHARED_LIBRARY)

