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

ifdef LIBPIDWATCH_FAUTES_SUPPORT
LOCAL_SRC_FILES += $(call all-c-files-under,tests)

LOCAL_LIBRARIES += libfautes
endif

include $(BUILD_SHARED_LIBRARY)
