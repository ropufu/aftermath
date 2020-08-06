
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_ARITHMETIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_ARITHMETIC_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/arithmetic.hpp"
#include "../core.hpp"

#include <cstdint> // std::int??_t, std::uint??_t
#include <limits>  // std::numeric_limits

#define ROPUFU_AFTERMATH_TESTS_ARITHMETIC_SIGNED_TYPES \
    std::int16_t, std::int32_t, std::int64_t           \

#define ROPUFU_AFTERMATH_TESTS_ARITHMETIC_UNSIGNED_TYPES \
    std::uint16_t, std::uint32_t, std::uint64_t          \

#define ROPUFU_AFTERMATH_TESTS_ARITHMETIC_ALL_TYPES  \
    ROPUFU_AFTERMATH_TESTS_ARITHMETIC_SIGNED_TYPES,  \
    ROPUFU_AFTERMATH_TESTS_ARITHMETIC_UNSIGNED_TYPES \


TEST_CASE_TEMPLATE("testing positive integer arithmetic", integer_type, ROPUFU_AFTERMATH_TESTS_ARITHMETIC_ALL_TYPES)
{
    integer_type a = 0;

    SUBCASE("small") { a = 10; }
    SUBCASE("max over 5") { a = std::numeric_limits<integer_type>::max() / 5; }
    SUBCASE("max over 3") { a = std::numeric_limits<integer_type>::max() / 3; }
    SUBCASE("max over 2") { a = std::numeric_limits<integer_type>::max() / 2; }
    // SUBCASE("max over 2 plus 1") { a = std::numeric_limits<integer_type>::max() / 2; ++a; } // Fails.

    CAPTURE(a);

    constexpr integer_type zero = 0;
    constexpr integer_type one = 1;
    integer_type b = static_cast<integer_type>(a + one);

    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(a, a), one);
    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(a, b), zero);
    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(one, b), zero);

    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(a, a), one);
    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(a, b), one);
    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(one, b), one);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing negative integer arithmetic", integer_type, ROPUFU_AFTERMATH_TESTS_ARITHMETIC_SIGNED_TYPES)
{
    integer_type a = 0;

    SUBCASE("small") { a = 10; }
    SUBCASE("max over 5") { a = std::numeric_limits<integer_type>::max() / 5; }
    SUBCASE("max over 3") { a = std::numeric_limits<integer_type>::max() / 3; }
    SUBCASE("max over 2") { a = std::numeric_limits<integer_type>::max() / 2; }
    // SUBCASE("max over 2 plus 1") { a = std::numeric_limits<integer_type>::max() / 2; ++a; } // Fails.

    CAPTURE(a);

    constexpr integer_type zero = 0;
    constexpr integer_type one = 1;
    constexpr integer_type minus_one = -1;
    integer_type minus_a = -a;
    integer_type b = static_cast<integer_type>(a + one);

    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(minus_a, a), minus_one);
    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(minus_a, b), zero);
    CHECK_EQ(ropufu::aftermath::fraction_toward_zero(minus_one, b), zero);

    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(minus_a, a), minus_one);
    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(minus_a, b), minus_one);
    CHECK_EQ(ropufu::aftermath::fraction_away_from_zero(minus_one, b), minus_one);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_ARITHMETIC_HPP_INCLUDED
