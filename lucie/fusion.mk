OLD_PACKAGE_DIR:=$(PACKAGE_DIR)


PACKAGE_DIR:=$(OLD_PACKAGE_DIR)/fusion

# find all the .mk files which are a in lucie directory, itself in a
# subdirectory of external
LUCIE_MK:=$(shell find $(PACKAGE_DIR)/*/ -mindepth 2 -name *.mk -path */lucie/*.mk)

# macro for retrieving the path of the current makefile
# useful to be independent of tree structure
my-dir = $(abspath $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

include $(LUCIE_MK)

PACKAGE_DIR:=$(OLD_PACKAGE_DIR)
