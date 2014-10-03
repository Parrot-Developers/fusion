/**
 * @file ut_module.h
 * @brief utilities for loading linux kernel modules and checking their presence
 *
 * @date 1 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_MODULE_H_
#define UT_MODULE_H_
#include <stdbool.h>

/**
 * @def UT_MODULE_MAX_FILTER_CRITERIA
 * @brief maximum number of filter criteria an ut_module_binding can hold
 */
#define UT_MODULE_MAX_FILTER_CRITERIA 7

/**
 * @struct ut_module_binding
 * @brief describes a device, by specifying a filter criteria list, which can be
 * concatenated an fed into a "new_id" file for usb and pci buses
 * @note there can be no "hole" in the criteria list definition, e.g., when
 * defining an usb binding, if on defines the id_vendor and b_interface_class
 * criteria, then the id_product criterium must be defined and valid too. The
 * reason is that if id_product was empty, then b_interface_class would be
 * mistaken for it. Please note that bindings with holes are rejected by
 * ut_module_bindings_are_valid(), ut_modules_apply_bindings()...
 */
struct ut_module_binding {
	/**
	 * anonymous union for being independent of the underlying bus type.
	 * for declaring a usb module binding, use usb_filters, for a pci module
	 * binding, use pci_filters. In both cases, don't forget to memset the
	 * structure prior to setting it's fields, or at least, for the usb
	 * case, set the 'must_be_null' to NULL
	 */
	union {
		/**
		 * for internal use only, allows concatenating the criteria in a
		 * simple way
		 */
		const char const *gen_filters[UT_MODULE_MAX_FILTER_CRITERIA];
		/**
		 * matching criteria for an usb module
		 * @see kernel/Documentation/ABI/testing/sysfs-bus-usb: new_id
		 */
		struct {
			const char const *id_vendor;
			const char const *id_product;
			const char const *b_interface_class;
			const char const *ref_id_vendor;
			const char const *ref_id_product;
			const char const *must_be_null;
		} usb_filters;
		/**
		 * matching criteria for an pci module
		 * @see kernel/Documentation/ABI/testing/sysfs-bus-pci: new_id
		 */
		struct {
			const char const *vendor_id;
			const char const *device_id;
			const char const *subsystem_vendor_id;
			const char const *subsystem_device_id;
			const char const *class;
			const char const *class_mask;
			const char const *private_driver_data;
		} pci_filters;
	};
};

/**
 * @struct ut_module
 * @brief structure representing a linux kernel module
 */
struct ut_module {
	/** module name, as if passed to modprobe */
	const char *name;
	/** file for manual binding a device */
	const char *new_id_path;
	/** list of devices needed to be bound to the driver at startup */
	const struct ut_module_binding *bindings;
};

/**
 * Load one module, with all it's dependencies
 * @param module to load
 * @return -EINVAL on error, 0 on success
 */
int ut_module_load(const struct ut_module *module);

/**
 * loads a list of modules
 * @param list the list of the modules to load, the last one must have all it's
 * fields set to NULL
 * @return errno-compatible negative value on error, 0 on success
 */
int ut_modules_load(const struct ut_module *list);

/**
 * Check if a module is loaded, that is either built as a module and loaded, or
 * built in the kernel
 * @param module module to check the presence of
 * @return true iif the module is loaded
 */
bool ut_module_is_loaded(const struct ut_module *module);

/* TODO ut_module_exists(const struct ut_module);, or (const char *) */

/**
 * Checks if a list of modules, in the same way as ut_module_is_loaded() does
 * @see ut_module_is_loaded()
 * @param list List of the modules to check, the last one must have all it's
 * fields set to NULL
 * @return true iif all the drivers are loaded
 */
bool ut_modules_are_loaded(const struct ut_module *list);

/**
 * Bind all the devices having a binding in the binding field of the module,
 * to the module's driver
 * @param module module to apply the bindings of
 * @return errno-compatible negative value on error, 0 on success
 */
int ut_module_apply_bindings(const struct ut_module *module);

/**
 * Same as ut_module_apply_bindings(), but for a list of modules
 * @see ut_module_apply_bindings
 * @param list List of the modules to apply the bindings of, the last one must
 * have all it's fields set to NULL
 * @return errno-compatible negative value on error, 0 on success
 */
int ut_modules_apply_bindings(const struct ut_module *list);

/**
 * Check if a module is present in a list of modules
 * @param list List of modules, the module will be looked for into
 * @param name name of the module to check
 * @return true iif the module belongs to the list
 */
bool ut_modules_contains(const struct ut_module *list, const char *name);

/**
 * Check if all the bindings of a module are valid, that is, if they contain no
 * "hole" in their matching criteria list, if the required criteria are present
 * or if the list is empty
 * @param module Module to test the bindings of
 * @return true iif the bindings are valid
 */
bool ut_module_bindings_are_valid(const struct ut_module *module);

#endif /* UT_MODULE_H_ */
