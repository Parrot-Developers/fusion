###############################################################################
# auto-generated lucie makefile from atom.mk with alchemy2lucie.py version 0.1
# DO NOT EDIT BY HAND OR A PUPPY WILL BE KILLED !!!
# I AM SERIOUS !!!
#
# @author nicolas.carrier@parrot.com
#
# libfautes exported from libfautes alchemy module
###############################################################################

PACKAGE_NAME = libfautes
PACKAGE_DEPS = uclibc libcunit

LIBFAUTES_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)/
LIBFAUTES_PACKAGE_DIR := $(call my-dir)/../

LIBFAUTES_OBJ_FILES := $(LIBFAUTES_BUILD_DIR)/lib/fautes_utils.c.o

$(PACKAGE_NAME): $(PACKAGE_DEPS) $(LIBFAUTES_BUILD_DIR)/libfautes.so
	@echo compiled $@
	$(Q) mkdir -p  $(STAGING_DIR)/usr/lib/
	$(Q) mkdir -p  $(TARGET_DIR)/usr/lib/
	$(Q) install -m0644 $(LIBFAUTES_BUILD_DIR)/libfautes.so $(STAGING_DIR)/usr/lib/libfautes.so
	$(Q) install -m0644 $(LIBFAUTES_BUILD_DIR)/libfautes.so $(TARGET_DIR)/usr/lib/libfautes.so
	$(Q) mkdir -p  $(STAGING_DIR)/usr/include/libfautes/
	$(Q) cp -R $(LIBFAUTES_PACKAGE_DIR)/include $(STAGING_DIR)/usr/include/libfautes/
	@echo installed $@

$(LIBFAUTES_BUILD_DIR)/libfautes.so: $(LIBFAUTES_OBJ_FILES)
	$(Q) mkdir -p $(dir $@)
	$(Q) $(TARGET_CXX) \
		$(TARGET_GLOBAL_LDFLAGS_SHARED) \
		-Wl,-Map -Wl,$(basename $@).map \
		-shared \
		-Wl,-soname -Wl,$(notdir $@) \
		-Wl,--no-undefined \
		-Wl,--gc-sections \
		-Wl,--as-needed \
		$(LDFLAGS) \
		$(LIBFAUTES_OBJ_FILES) \
		-Wl,--no-whole-archive \
		 \
		-o $@ \
		-lcunit -lcunit \
		$(TARGET_DYN_LDFLAGS)

$(LIBFAUTES_BUILD_DIR)/lib/fautes_utils.c.o: $(LIBFAUTES_PACKAGE_DIR)/lib/fautes_utils.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBFAUTES_BUILD_DIR) \
		-I$(LIBFAUTES_PACKAGE_DIR) \
		-I$(LIBFAUTES_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(PACKAGE_NAME)-clean:
	$(Q) $(RM) -f $(TARGET_DIR)/usr/lib/libfautes.so
	$(Q) $(RM) -f $(STAGING_DIR)/usr/lib/libfautes.so

$(PACKAGE_NAME)-dirclean: $(PACKAGE_NAME)-clean
	$(Q) $(RM) -Rf $(LIBFAUTES_BUILD_DIR)/

###############################################################################
#
# Toplevel Makefile options
#
###############################################################################
TARGETS-$(BR2_PACKAGE_LIBFAUTES)+=$(PACKAGE_NAME)
