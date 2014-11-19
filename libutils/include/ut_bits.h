/**
 * @file ut_bits.h
 * @brief bit manipulation and test macros
 *
 * @date 17 nov. 2014
 * @author nicolas.carrier@parrot.com
 * @copyright Copyright (C) 2014 Parrot S.A.
 */
#ifndef UT_BITS_H_
#define UT_BITS_H_

/**
 * @def UT_LOW_NIBBLE
 * @brief extracts the low nibble of a number
 */
#define UT_LOW_NIBBLE(c) (0x0F & (c))

/**
 * @def UT_HIGH_NIBBLE
 * @brief extracts the high nibble of a number
 */
#define UT_HIGH_NIBBLE(c) (UT_LOW_NIBBLE((c) >> 4))

/**
 * @def UT_BIT
 * @brief creates an integer with only the b-th bit set
 */
#define UT_BIT(b) (1 << (b))

/**
 * @def UT_TO_LOWER
 * @brief sets the fifth bit, that is, if the number is in the range ['A', 'Z'],
 * put it in ['a', 'z']
 * @note doesn't check if c really is in ['A', 'Z']
 */
#define UT_TO_LOWER(c) ((c) | UT_BIT(5))

/**
 * @def UT_TO_UPPER
 * @brief unsets the fifth bit, that is, if the number is in the range
 * ['a', 'z'],  put it in ['A', 'Z']
 * @note doesn't check if c really is in ['a', 'z']
 */
#define UT_TO_UPPER(c) ((c) & ~UT_BIT(5))

/**
 * @def UT_IS_AZ_LOWER
 * @brief checks if the number is in the ['a', 'z'] range
 */
#define UT_IS_AZ_LOWER(c) ((c) <= 'z' && (c) >= 'a')

/**
 * @def UT_IS_09
 * @brief checks if the number is in the ['0', '9'] range
 */
#define UT_IS_09(c) ((c) <= '9' && (c) >= '0')

/**
 * @def UT_IS_AZ_NO_CASE
 * @brief checks if the number is in the ['A', 'Z'], or in the ['a', 'z'] range
 */
#define UT_IS_AZ_NO_CASE(c) (UT_IS_AZ_LOWER(UT_TO_LOWER(c)))

/**
 * @def UT_IS_AZ_NO_CASE
 * @brief checks if the number is in ['A', 'Z'] U ['a', 'z'] U ['0', '9']
 */
#define UT_IS_09_OR_AZ_NO_CASE(c) (UT_IS_09((c)) || UT_IS_AZ_NO_CASE((c)))

#endif /* UT_BITS_H_ */
