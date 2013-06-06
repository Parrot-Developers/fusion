###############################################################################
# auto-generated lucie makefile from atom.mk with alchemy2lucie.py version 0.1
# DO NOT EDIT BY HAND OR A PUPPY WILL BE KILLED !!!
# I AM SERIOUS !!!
#
# @author nicolas.carrier@parrot.com
#
# fautes exported from fautes alchemy module
###############################################################################

PACKAGE_NAME = fautes
PACKAGE_DEPS = uclibc libfautes libcunit

FAUTES_BUILD_DIR := $(BUILD_DIR)/$(PACKAGE_NAME)/
FAUTES_PACKAGE_DIR := $(call my-dir)/../

FAUTES_OBJ_FILES := $(FAUTES_BUILD_DIR)/src/fautes.c.o

$(PACKAGE_NAME): $(PACKAGE_DEPS) $(FAUTES_BUILD_DIR)/fautes
	@echo compiled $@
	$(Q) mkdir -p  $(STAGING_DIR)/usr/bin/
	$(Q) mkdir -p  $(TARGET_DIR)/usr/bin/
	$(Q) install -m0755 $(FAUTES_BUILD_DIR)/fautes $(STAGING_DIR)/usr/bin/fautes
	$(Q) install -m0755 $(FAUTES_BUILD_DIR)/fautes $(TARGET_DIR)/usr/bin/fautes
	@echo installed $@

$(FAUTES_BUILD_DIR)/fautes: $(FAUTES_OBJ_FILES)
	$(Q) mkdir -p $(dir $@)
	$(Q) $(TARGET_CXX) \
		$(TARGET_GLOBAL_LDFLAGS) \
		$(LDFLAGS) \
		$(FAUTES_OBJ_FILES) \
		-Wl,--no-whole-archive \
		$(STAGING_DIR)/usr/lib/libfautes.so \
		-o $@ \
		-ldl -lcunit -lcunit \
		$(TARGET_LDFLAGS)

$(FAUTES_BUILD_DIR)/src/fautes.c.o: $(FAUTES_PACKAGE_DIR)/src/fautes.c
	$(Q) mkdir -p $(dir $@)
	$(TARGET_CC) -I$(FAUTES_BUILD_DIR) \
		-I$(FAUTES_PACKAGE_DIR) \
		-I$(FAUTES_PACKAGE_DIR)/include \
		-I$(STAGING_DIR)/usr/include/libfautes/include \
		-I$(STAGING_DIR)/usr/include/libcunit \
		-I$(STAGING_DIR)/usr/include/\
		$(TARGET_GLOBAL_CFLAGS) \
		-DBUILD_LIBFAUTES -DBUILD_LIBCUNIT -O0 -g -c -MMD -MP -o $@ $<

$(PACKAGE_NAME)-clean:
	$(Q) $(RM) -f $(TARGET_DIR)/usr/bin/fautes
	$(Q) $(RM) -f $(STAGING_DIR)/usr/bin/fautes

$(PACKAGE_NAME)-dirclean: $(PACKAGE_NAME)-clean
	$(Q) $(RM) -Rf $(FAUTES_BUILD_DIR)/

###############################################################################
#
# Toplevel Makefile options
#
###############################################################################
TARGETS-$(BR2_PACKAGE_FAUTES)+=$(PACKAGE_NAME)
