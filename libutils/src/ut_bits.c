/**
 * @file ut_bits.c
 * @brief
 *
 * @date May 20, 2015
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2015 Parrot S.A.
 */
#include <errno.h>
#include <stdlib.h>
#include <inttypes.h>

#include <ut_bits.h>

uint8_t ut_bit_field_claim_free_index(ut_bit_field_t *indices)
{
	uint8_t i;
	ut_bit_field_t bit;
	uint8_t max = UT_BIT_FIELD_MAX - 1;

	if (indices == NULL)
		return -EINVAL;

	for (i = 0; i < max; i++) {
		bit = (1 << i);
		if ((*indices & bit) == 0) {
			/* claim the index */
			*indices |= bit;
			return i;
		}
	}

	/* no free index */
	return UT_BIT_FIELD_INVALID_INDEX;
}

int ut_bit_field_release_index(ut_bit_field_t *indices, uint8_t index)
{
	if (index >= UT_BIT_FIELD_MAX || indices == NULL)
		return -EINVAL;

	*indices &= ~(1 << index);

	return 0;
}
