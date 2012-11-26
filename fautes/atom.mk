LOCAL_PATH := $(call my-dir)

###############################################################################
# libfautes
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := libfautes

LOCAL_EXPORT_C_INCLUDES  := $(LOCAL_PATH)/include

LOCAL_EXPORT_LDLIBS := -lcunit

LOCAL_SRC_FILES := \
	$(call all-c-files-under,lib,.c) \

LOCAL_LIBRARIES := libcunit

include $(BUILD_SHARED_LIBRARY)

###############################################################################
# fautes
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE := fautes

LOCAL_SRC_FILES := \
	$(call all-c-files-under,src,.c) \

LOCAL_LDLIBS := -ldl

LOCAL_LIBRARIES := libfautes libcunit

include $(BUILD_EXECUTABLE)


