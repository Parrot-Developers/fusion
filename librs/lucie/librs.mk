###############################################################################
# auto-generated lucie makefile from atom.mk with alchemy2lucie.py version 0.1
# DO NOT EDIT BY HAND OR A PUPPY WILL BE KILLED !!!
# I AM SERIOUS !!!
#
# @author nicolas.carrier@parrot.com
#
# librs exported from librs alchemy module
###############################################################################

PACKAGE_NAME = librs
PACKAGE_DEPS = uclibc libfautes libcunit

LIBRS_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)/
LIBRS_PACKAGE_DIR := $(call my-dir)/../

LIBRS_OBJ_FILES := $(LIBRS_BUILD_DIR)/src/rs_rb.c.o \
	$(LIBRS_BUILD_DIR)/src/rs_dll.c.o \
	$(LIBRS_BUILD_DIR)/src/rs_hmap.c.o \
	$(LIBRS_BUILD_DIR)/src/rs_node.c.o \
	$(LIBRS_BUILD_DIR)/tests/rs_fautes.c.o \
	$(LIBRS_BUILD_DIR)/tests/rs_node_test.c.o \
	$(LIBRS_BUILD_DIR)/tests/rs_hmap_test.c.o \
	$(LIBRS_BUILD_DIR)/tests/rs_dll_test.c.o \
	$(LIBRS_BUILD_DIR)/tests/rs_rb_test.c.o

$(PACKAGE_NAME): $(PACKAGE_DEPS) $(LIBRS_BUILD_DIR)/librs.so
	@echo compiled $@
	$(Q) mkdir -p  $(STAGING_DIR)/usr/lib/
	$(Q) mkdir -p  $(TARGET_DIR)/usr/lib/
	$(Q) install -m0644 $(LIBRS_BUILD_DIR)/librs.so $(STAGING_DIR)/usr/lib/librs.so
	$(Q) install -m0644 $(LIBRS_BUILD_DIR)/librs.so $(TARGET_DIR)/usr/lib/librs.so
	$(Q) mkdir -p  $(STAGING_DIR)/usr/include/librs/
	$(Q) cp -R $(LIBRS_PACKAGE_DIR)/include $(STAGING_DIR)/usr/include/librs/
	@echo installed $@

$(LIBRS_BUILD_DIR)/librs.so: $(LIBRS_OBJ_FILES)
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
		$(LIBRS_OBJ_FILES) \
		-Wl,--no-whole-archive \
		$(STAGING_DIR)/usr/lib/libfautes.so \
		-o $@ \
		-lcunit \
		$(TARGET_DYN_LDFLAGS)

$(LIBRS_BUILD_DIR)/src/rs_rb.c.o: $(LIBRS_PACKAGE_DIR)/src/rs_rb.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/src/rs_dll.c.o: $(LIBRS_PACKAGE_DIR)/src/rs_dll.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/src/rs_hmap.c.o: $(LIBRS_PACKAGE_DIR)/src/rs_hmap.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/src/rs_node.c.o: $(LIBRS_PACKAGE_DIR)/src/rs_node.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/tests/rs_fautes.c.o: $(LIBRS_PACKAGE_DIR)/tests/rs_fautes.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/tests/rs_node_test.c.o: $(LIBRS_PACKAGE_DIR)/tests/rs_node_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/tests/rs_hmap_test.c.o: $(LIBRS_PACKAGE_DIR)/tests/rs_hmap_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/tests/rs_dll_test.c.o: $(LIBRS_PACKAGE_DIR)/tests/rs_dll_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBRS_BUILD_DIR)/tests/rs_rb_test.c.o: $(LIBRS_PACKAGE_DIR)/tests/rs_rb_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBRS_BUILD_DIR) \
		-I$(LIBRS_PACKAGE_DIR) \
		-I$(LIBRS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(PACKAGE_NAME)-clean:
	$(Q) $(RM) -f $(TARGET_DIR)/usr/lib/librs.so
	$(Q) $(RM) -f $(STAGING_DIR)/usr/lib/librs.so

$(PACKAGE_NAME)-dirclean: $(PACKAGE_NAME)-clean
	$(Q) $(RM) -Rf $(LIBRS_BUILD_DIR)/

###############################################################################
#
# Toplevel Makefile options
#
###############################################################################
TARGETS-$(BR2_PACKAGE_LIBRS)+=$(PACKAGE_NAME)
