###############################################################################
# auto-generated lucie makefile from atom.mk with alchemy2lucie.py version 0.1
# DO NOT EDIT BY HAND OR A PUPPY WILL BE KILLED !!!
# I AM SERIOUS !!!
#
# @author nicolas.carrier@parrot.com
#
# libpidwatch exported from libpidwatch alchemy module
###############################################################################

PACKAGE_NAME = libpidwatch
PACKAGE_DEPS = uclibc libfautes libcunit

LIBPIDWATCH_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)/
LIBPIDWATCH_PACKAGE_DIR := $(call my-dir)/../

LIBPIDWATCH_OBJ_FILES := $(LIBPIDWATCH_BUILD_DIR)/src/pidwatch.c.o \
	$(LIBPIDWATCH_BUILD_DIR)/tests/pw_fautes.c.o \
	$(LIBPIDWATCH_BUILD_DIR)/tests/pw_tests.c.o

$(PACKAGE_NAME): $(PACKAGE_DEPS) $(LIBPIDWATCH_BUILD_DIR)/libpidwatch.so
	@echo compiled $@
	$(Q) mkdir -p  $(STAGING_DIR)/usr/lib/
	$(Q) mkdir -p  $(TARGET_DIR)/usr/lib/
	$(Q) install -m0644 $(LIBPIDWATCH_BUILD_DIR)/libpidwatch.so $(STAGING_DIR)/usr/lib/libpidwatch.so
	$(Q) install -m0644 $(LIBPIDWATCH_BUILD_DIR)/libpidwatch.so $(TARGET_DIR)/usr/lib/libpidwatch.so
	$(Q) mkdir -p  $(STAGING_DIR)/usr/include/libpidwatch/
	$(Q) cp -R $(LIBPIDWATCH_PACKAGE_DIR)/include $(STAGING_DIR)/usr/include/libpidwatch/
	@echo installed $@

$(LIBPIDWATCH_BUILD_DIR)/libpidwatch.so: $(LIBPIDWATCH_OBJ_FILES)
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
		$(LIBPIDWATCH_OBJ_FILES) \
		-Wl,--no-whole-archive \
		$(STAGING_DIR)/usr/lib/libfautes.so \
		-o $@ \
		-lcunit \
		$(TARGET_DYN_LDFLAGS)

$(LIBPIDWATCH_BUILD_DIR)/src/pidwatch.c.o: $(LIBPIDWATCH_PACKAGE_DIR)/src/pidwatch.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBPIDWATCH_BUILD_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBPIDWATCH_BUILD_DIR)/tests/pw_fautes.c.o: $(LIBPIDWATCH_PACKAGE_DIR)/tests/pw_fautes.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBPIDWATCH_BUILD_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBPIDWATCH_BUILD_DIR)/tests/pw_tests.c.o: $(LIBPIDWATCH_PACKAGE_DIR)/tests/pw_tests.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBPIDWATCH_BUILD_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR) \
		-I$(LIBPIDWATCH_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(PACKAGE_NAME)-clean:
	$(Q) $(RM) -f $(TARGET_DIR)/usr/lib/libpidwatch.so
	$(Q) $(RM) -f $(STAGING_DIR)/usr/lib/libpidwatch.so

$(PACKAGE_NAME)-dirclean: $(PACKAGE_NAME)-clean
	$(Q) $(RM) -Rf $(LIBPIDWATCH_BUILD_DIR)/

###############################################################################
#
# Toplevel Makefile options
#
###############################################################################
TARGETS-$(BR2_PACKAGE_LIBPIDWATCH)+=$(PACKAGE_NAME)
