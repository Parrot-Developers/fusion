###############################################################################
# auto-generated lucie makefile from atom.mk with alchemy2lucie.py version 0.1
# DO NOT EDIT BY HAND OR A PUPPY WILL BE KILLED !!!
# I AM SERIOUS !!!
#
# @author nicolas.carrier@parrot.com
#
# libioutils exported from libioutils alchemy module
###############################################################################

PACKAGE_NAME = libioutils
PACKAGE_DEPS = uclibc librs libpidwatch libfautes libcunit

LIBIOUTILS_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)/
LIBIOUTILS_PACKAGE_DIR := $(call my-dir)/../

LIBIOUTILS_OBJ_FILES := $(LIBIOUTILS_BUILD_DIR)/src/io_src_pid.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_mon.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src_msg_uad.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_utils.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src_tmr.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_platform.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src_sig.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src_sep.c.o \
	$(LIBIOUTILS_BUILD_DIR)/src/io_src_msg.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_msg_uad_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_mon_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_tmr_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_utils_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_sig_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_pid_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_msg_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_src_sep_test.c.o \
	$(LIBIOUTILS_BUILD_DIR)/tests/io_fautes.c.o

$(PACKAGE_NAME): $(PACKAGE_DEPS) $(LIBIOUTILS_BUILD_DIR)/libioutils.so
	@echo compiled $@
	$(Q) mkdir -p  $(STAGING_DIR)/usr/lib/
	$(Q) mkdir -p  $(TARGET_DIR)/usr/lib/
	$(Q) install -m0644 $(LIBIOUTILS_BUILD_DIR)/libioutils.so $(STAGING_DIR)/usr/lib/libioutils.so
	$(Q) install -m0644 $(LIBIOUTILS_BUILD_DIR)/libioutils.so $(TARGET_DIR)/usr/lib/libioutils.so
	$(Q) mkdir -p  $(STAGING_DIR)/usr/include/libioutils/
	$(Q) cp -R $(LIBIOUTILS_PACKAGE_DIR)/include $(STAGING_DIR)/usr/include/libioutils/
	@echo installed $@

$(LIBIOUTILS_BUILD_DIR)/libioutils.so: $(LIBIOUTILS_OBJ_FILES)
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
		$(LIBIOUTILS_OBJ_FILES) \
		-Wl,--no-whole-archive \
		$(STAGING_DIR)/usr/lib/librs.so $(STAGING_DIR)/usr/lib/libpidwatch.so $(STAGING_DIR)/usr/lib/libfautes.so \
		-o $@ \
		-lcunit \
		$(TARGET_DYN_LDFLAGS)

$(LIBIOUTILS_BUILD_DIR)/src/io_src_pid.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_pid.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_mon.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_mon.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src_msg_uad.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_msg_uad.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_utils.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_utils.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src_tmr.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_tmr.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_platform.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_platform.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src_sig.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_sig.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src_sep.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_sep.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/src/io_src_msg.c.o: $(LIBIOUTILS_PACKAGE_DIR)/src/io_src_msg.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_msg_uad_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_msg_uad_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_mon_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_mon_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_tmr_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_tmr_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_utils_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_utils_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_sig_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_sig_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_pid_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_pid_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_msg_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_msg_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_src_sep_test.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_src_sep_test.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(LIBIOUTILS_BUILD_DIR)/tests/io_fautes.c.o: $(LIBIOUTILS_PACKAGE_DIR)/tests/io_fautes.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(LIBIOUTILS_BUILD_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR) \
		-I$(LIBIOUTILS_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/librs/include \
		-I$(STAGING_DIR)/usr/include/libpidwatch/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBRS -DBUILD_LIBPIDWATCH -DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $< -fPIC

$(PACKAGE_NAME)-clean:
	$(Q) $(RM) -f $(TARGET_DIR)/usr/lib/libioutils.so
	$(Q) $(RM) -f $(STAGING_DIR)/usr/lib/libioutils.so

$(PACKAGE_NAME)-dirclean: $(PACKAGE_NAME)-clean
	$(Q) $(RM) -Rf $(LIBIOUTILS_BUILD_DIR)/

###############################################################################
#
# Toplevel Makefile options
#
###############################################################################
TARGETS-$(BR2_PACKAGE_LIBIOUTILS)+=$(PACKAGE_NAME)
