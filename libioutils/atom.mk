LOCAL_PATH := $(call my-dir)

###############################################################################
# libioutils
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libioutils
LOCAL_DESCRIPTION := Utils for event loop based asynchronous I/O
LOCAL_CATEGORY_PATH := libs

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(call all-c-files-under,src,.c)

LOCAL_LIBRARIES := librs

ifdef LIBIOUTILS_FAUTES_SUPPORT
LOCAL_SRC_FILES += $(call all-c-files-under,tests,.c)

LOCAL_LIBRARIES += libfautes
endif

include $(BUILD_SHARED_LIBRARY)

