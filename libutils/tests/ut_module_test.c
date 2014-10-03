/**
 * @file ut_module_test.c
 * @date 3 oct. 2014
 * @author nicolas.carrier@parrot.com
 * @brief unit tests for libutils' module module
 *
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#include <CUnit/Basic.h>

#include "../include/ut_module.h"
#include "../include/ut_file.h"

#include <fautes.h>
#include <fautes_utils.h>

/**
 * Reads the list of the modules listed in /proc/modules and store their names
 * in an array
 * @param module in output, contains the list of modules read terminated with an
 * empty string (and followed by NULL pointers)
 * @return number of modules in the list
 */
static int read_modules_list(char *module[0x100])
{
	int ret;
	char *c;
	char *proc_modules;
	int nb_modules = 0;

	ret = ut_file_to_string("/proc/modules", &proc_modules);
	CU_ASSERT_EQUAL(ret, 0);
	*module = proc_modules;
	module++;
	for (c = proc_modules; *c != '\0'; c++) {
		if (*c == ' ') {
			*c = '\0';
			c++;
			/* skip the rest of the line */
			while (*c != '\n' && *c != '\0')
				c++;
			if (*c == '\0')
				break;
			c++;
			*module = c;
			nb_modules++;
			module++;
		}
	}

	return nb_modules;
}

static void testUT_MODULE_LOAD(void)
{
	int ret;
	static struct ut_module module;

	/* TODO don't have any clue on how to test that... */

	/* error use cases */
	module.name = "improbable_module_name";
	ret = ut_module_load(&module);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	module.name = "";
	ret = ut_module_load(&module);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	module.name = NULL;
	ret = ut_module_load(&module);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_module_load(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

static void testUT_MODULES_LOAD(void)
{
	int ret;
	static struct ut_module list[2];

	/* TODO don't have any clue on how to test that... */

	/* error use cases */
	list[0].name = "improbable_module_name";
	ret = ut_modules_load(list);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	list[0].name = "";
	ret = ut_modules_load(list);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	list[0].name = NULL;
	ret = ut_modules_load(list);
	CU_ASSERT_NOT_EQUAL(ret, 0);
	ret = ut_modules_load(NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);


}

static void testUT_MODULE_IS_LOADED(void)
{
	struct ut_module built_in_module = {
			/* assume we'll always have usb */
			.name = "usbcore",
	};
	struct ut_module improbable_module = {
			/* assume we'll always have usb */
			.name = "ursule",
	};
	struct ut_module module;

	/* normal use cases */
	/* check usb_core is "loaded" */
	CU_ASSERT(ut_module_is_loaded(&built_in_module));
	/* check an improbable module isn't loaded */
	CU_ASSERT(!ut_module_is_loaded(&improbable_module));

	/* error use cases */
	module.name = "";
	CU_ASSERT_FALSE(ut_module_is_loaded(&module));
	module.name = NULL;
	CU_ASSERT_FALSE(ut_module_is_loaded(&module));
	CU_ASSERT_FALSE(ut_module_is_loaded(NULL));
}

static void testUT_MODULES_ARE_LOADED(void)
{
	char *modules[0x100] = {NULL};
	struct ut_module *list = NULL;
	int nb_modules;
	/* use static so that all is set to 0 (especially .must_be_null) */
	/* required_size + 1 so that there is a last element all NULL */
	static const struct ut_module built_in_module[2] = {
			{
					/* assume we'll always have usb */
					.name = "usbcore",
			}
	};
	static const struct ut_module improbable_list[2] = {
			{
					/* assume we'll always have usb */
					.name = "ursule",
			}
	};
	static const struct ut_module empty_list[1] = {};

	nb_modules = read_modules_list(modules);
	list = calloc(nb_modules + 1, sizeof(*list));
	CU_ASSERT_PTR_NOT_NULL_FATAL(list);
	while (--nb_modules)
		list->name = modules[nb_modules];

	/* normal use cases */
	/* check all the modules listed in /proc/modules are "loaded" */
	CU_ASSERT(ut_modules_are_loaded(list));
	/* check usb_core is "loaded" */
	CU_ASSERT(ut_modules_are_loaded(built_in_module));
	/* check an improbable module isn't loaded */
	CU_ASSERT(!ut_modules_are_loaded(improbable_list));
	/* an empty list should return true */
	CU_ASSERT(ut_modules_are_loaded(empty_list));

	/* error use cases */
	CU_ASSERT_FALSE(ut_modules_are_loaded(NULL));
}

static void testUT_MODULES_CONTAINS(void)
{
	/* use static so that all is set to 0 (especially .must_be_null) */
	/* required_size + 1 so that there is a last element all NULL */
	static const struct ut_module list[3] = {
			{
					.name = "tata",
			},
			{
					.name = "toto",
			},
	};
	static const struct ut_module empty_modules[1];

	/* normal use cases */
	CU_ASSERT(ut_modules_contains(list, "tata"));
	CU_ASSERT(ut_modules_contains(list, "toto"));
	CU_ASSERT(!ut_modules_contains(list, "titi"));
	CU_ASSERT(!ut_modules_contains(empty_modules, "titi"));

	/* error use cases */
	CU_ASSERT(!ut_modules_contains(NULL, "titi"));
	CU_ASSERT(!ut_modules_contains(list, ""));
	CU_ASSERT(!ut_modules_contains(list, NULL));
}

static void testUT_MODULE_BINDINGS_ARE_VALID(void)
{
	/* use static so that all is set to 0 (especially .must_be_null) */
	/* required_size + 1 so that there is a last element all NULL */
	static const struct ut_module_binding valid_bindings[3] = {
		{
			.usb_filters = {
				.id_vendor = "tata",
				.id_product = "toto",
			}
		},
		{
			.usb_filters = {
				.id_vendor = "tata",
				.id_product = "toto",
				.b_interface_class = "a",
			}
		},
	};
	static const struct ut_module_binding invalid_bindings1[2] = {
		{
			.usb_filters = {
				.id_vendor = "tata",
				/* id_product is missing */
			}
		},
	};
	static const struct ut_module_binding invalid_bindings2[2] = {
		{
			.usb_filters = {
				.id_vendor = "tata",
				.id_product = "toto",
				/* b_interface_class is NULL: "hole" */
				.ref_id_vendor = "a",
			}
		},
	};
	static struct ut_module module = {
			.bindings = valid_bindings,
	};

	/* normal use cases */
	CU_ASSERT(ut_module_bindings_are_valid(&module));

	/* error use cases */
	module.bindings = invalid_bindings1;
	CU_ASSERT(!ut_module_bindings_are_valid(&module));
	module.bindings = invalid_bindings2;
	CU_ASSERT(!ut_module_bindings_are_valid(&module));
}

static const struct test_t tests[] = {
		{
				.fn = testUT_MODULE_LOAD,
				.name = "ut_module_load"
		},
		{
				.fn = testUT_MODULES_LOAD,
				.name = "ut_modules_load"
		},
		{
				.fn = testUT_MODULE_IS_LOADED,
				.name = "ut_module_is_loaded"
		},
		{
				.fn = testUT_MODULES_ARE_LOADED,
				.name = "ut_modules_are_loaded"
		},
		{
				.fn = testUT_MODULES_CONTAINS,
				.name = "ut_modules_contains"
		},
		{
				.fn = testUT_MODULE_BINDINGS_ARE_VALID,
				.name = "ut_module_bindings_are_valid"
		},

		/* NULL guard */
		{.fn = NULL, .name = NULL},
};

static int init_module_suite(void)
{
	return 0;
}

static int clean_module_suite(void)
{
	return 0;
}

struct suite_t module_suite = {
		.name = "ut_module",
		.init = init_module_suite,
		.clean = clean_module_suite,
		.tests = tests,
};
