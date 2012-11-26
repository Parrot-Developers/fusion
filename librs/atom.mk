LOCAL_PATH := $(call my-dir)

###############################################################################
# librs
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := librs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(call all-c-files-under,src,.c)

ifdef LIBRS_FAUTES_SUPPORT
LOCAL_SRC_FILES += $(call all-c-files-under,tests,.c)

LOCAL_LIBRARIES := libfautes
endif

include $(BUILD_SHARED_LIBRARY)

