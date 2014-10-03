/**
 * @file ut_module.c
 * @brief utilities for loading linux kernel modules and checking their presence
 *
 * @date 1 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <string.h>

#include "ut_module.h"
#include "ut_string.h"
#include "ut_log.h"
#include "ut_file.h"
#include "ut_process.h"

/**
 * @def MODPROBE_PROCFS
 * @brief path to the procfs file which should hold the path to the modprobe
 * utility
 */
#define MODPROBE_PROCFS "/proc/sys/kernel/modprobe"

/**
 * @def MODPROBE_DEFAULT_PATH
 * @brief default path if MODPROBE_PROCFS didn't provide something useful
 */
#define MODPROBE_DEFAULT_PATH "/sbin/modprobe"

/**
 * @var modprobe_path
 * @brief path to the modprobe utility, used for the modules loading
 */
static const char *modprobe_path = MODPROBE_DEFAULT_PATH;

/**
 * @var loaded_modprobe_path
 * @brief path to the modprobe utility, containet in the MODPROBE_PROCFS file
 */
static char *loaded_modprobe_path;

/**
 * constructor called at the library's loading, initializes the path to the
 * modprobe utility
 */
static void __attribute__ ((constructor)) ut_module_init(void)
{
	int ret;
	/* read the modprobe command line from /proc/sys/kernel/modprobe */

	ret = ut_file_to_string(MODPROBE_PROCFS, &loaded_modprobe_path);
	if (ret < 0 || !ut_file_is_executable(loaded_modprobe_path))
		ut_info(MODPROBE_PROCFS" gave no usable result, defaulting to "
				MODPROBE_DEFAULT_PATH);
	else
		modprobe_path = loaded_modprobe_path;
}

/**
 * destructor called at the library's unloading, frees the loaded modprobe path
 */
static void __attribute__ ((destructor)) ut_module_clean(void)
{
	ut_string_free(&loaded_modprobe_path);
	loaded_modprobe_path = MODPROBE_DEFAULT_PATH;
}
/**
 * Checks if a binding is valid, that is either if it is empty (NULL) or if it
 * has the required parameters, with no "holes"
 * @param binding list of binding criteria to be written to a module's new_id
 * file
 * @return true iif the binding is valid
 */
static bool binding_is_valid(const struct ut_module_binding *binding)
{
	int i;
	bool null_detected = false;

	/* empty binding list */
	if (binding->gen_filters[0] == NULL)
		return true;


	/* the first two are required for both pci and usb */
	if (binding->gen_filters[0] == NULL || binding->gen_filters[1] == NULL)
		return false;

	for (i = 0; i < UT_MODULE_MAX_FILTER_CRITERIA; i++) {
		if (!null_detected) {
			/* first NULL must be the end of the list */
			if (binding->gen_filters[i] == NULL) {
				null_detected = true;
				continue;
			}

			/* a filter is an invalid string: invalid */
			if (ut_string_is_invalid(binding->gen_filters[i]))
				return false;
		} else {
			/* there is a "hole" in the list: invalid */
			if (binding->gen_filters[i] != NULL)
				return false;
		}
	}

	return true;
}

/**
 * Bind a device to a driver, that is, write the list of the binding's criteria
 * to it's new_id file
 * @param new_id_path path to the module's new_id file
 * @param binding binding to apply
 * @return errno-compatible negative value on error, 0 on success
 */
static int bind_to_driver(const char *new_id_path,
		const struct ut_module_binding *binding)
{
	int i;
	int ret;
	int __attribute__((cleanup(ut_file_fd_close))) fd = -1;
	char __attribute__((cleanup(ut_string_free))) *str = NULL;

	/* new_id_path isn't always defined, so are the bindings */
	if (NULL == new_id_path || binding == NULL)
		return 0;

	fd = open(new_id_path, O_CLOEXEC | O_WRONLY);
	if (fd == -1) {
		ret = -errno;
		ut_err("open %s failed : %s", new_id_path, strerror(errno));
		return ret;
	}

	for (i = 0; i < UT_MODULE_MAX_FILTER_CRITERIA; i++) {
		ret = ut_string_append(&str, "%s ", binding->gen_filters[i]);
		if (ret < 0) {
			ut_perr("ut_string_append", ret);
			return ret;
		}
	}
	ret = write(fd, str, strlen(str));
	if (ret < 0) {
		ut_perr("write", ret);
		return ret;
	}

	return 0;
}

int ut_module_load(const struct ut_module *module)
{
	int ret;

	if (module == NULL || ut_string_is_invalid(module->name))
		return -EINVAL;

	if (ut_module_is_loaded(module))
		return 0;

	/*
	 * TODO try to use the init_module syscall, with or without the help of
	 * modules.dep
	 */
	ret = ut_process_vsystem("%s %s", modprobe_path, module->name);
	if (ret != 0) {
		ut_err("loading driver %s failed", module->name);
		return -EINVAL;
	}

	return 0;
}

int ut_modules_load(const struct ut_module *list)
{
	int ret;
	char __attribute__((cleanup(ut_string_free)))*module_list = NULL;

	if (list == NULL)
		return -EINVAL;

	while (list->name != NULL) {
		if (ut_string_is_invalid(list->name))
			return -EINVAL;

		ret = ut_string_append(&module_list, " %s", list->name);
		if (ret < 0) {
			ut_perr("ut_string_append", ret);
			return ret;
		}
		list++;
	}

	if (module_list == NULL)
		return -EINVAL;

	ret = ut_process_vsystem("%s -a %s", modprobe_path, module_list);
	if (ret != 0) {
		ut_err("ut_process_vsystem returned %d", ret);
		return -EINVAL;
	}

	return 0;
}

bool ut_module_is_loaded(const struct ut_module *module)
{
	int ret;
	struct stat sb;
	char *path __attribute__((cleanup(ut_string_free))) = NULL;

	if (module == NULL || ut_string_is_invalid(module->name))
		return false;

	ret = asprintf(&path, "/sys/module/%s", module->name);
	if (ret == -1) {
		ut_err("asprintf error");
		return false;
	}

	return stat(path, &sb) == 0;
}

bool ut_modules_are_loaded(const struct ut_module *list)
{
	if (list == NULL)
		return false;

	while (list->name != NULL)
		if (!ut_module_is_loaded(list))
			return false;
		else
			list++;

	return true;
}

int ut_module_apply_bindings(const struct ut_module *module)
{
	int ret;
	const struct ut_module_binding *binding;

	if (module == NULL || ut_string_is_invalid(module->new_id_path))
		return -EINVAL;

	/* empty binding list */
	if (module->bindings == NULL)
		return 0;

	if (!ut_module_bindings_are_valid(module))
		return -EINVAL;

	binding = module->bindings;
	while (binding->gen_filters[0] != NULL) {
		ret = bind_to_driver(module->new_id_path, binding);
		if (ret < 0) {
			ut_perr("bind_to_driver", ret);
			return ret;
		}
	}

	return 0;
}

int ut_modules_apply_bindings(const struct ut_module *list)
{
	int ret;

	if (list == NULL)
		return false;

	/* use the name for stop check, a module can have no binding */
	while (list->name != NULL) {
		ret = ut_module_apply_bindings(list++);
		if (ret < 0)
			return ret;
	}

	return 0;
}

bool ut_modules_contains(const struct ut_module *list, const char *name)
{
	if (ut_string_is_invalid(name) || list == NULL)
		return false;

	while (!ut_string_is_invalid(list->name))
		if (strcmp(list->name, name) == 0)
			return true;
		else
			list++;

	return false;
}

bool ut_module_bindings_are_valid(const struct ut_module *module)
{
	const struct ut_module_binding *binding = module->bindings;

	if (module == NULL)
		return false;

	while (binding->gen_filters[0] != NULL)
		if (!binding_is_valid(binding))
			return false;
		else
			binding++;

	return true;
}
