
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/algebra/fraction.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int16_t, std::int32_t, std::int64_t
#include <functional> // std::hash
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES \
    ropufu::aftermath::algebra::fraction<std::size_t>,    \
    ropufu::aftermath::algebra::fraction<std::uint16_t>,  \
    ropufu::aftermath::algebra::fraction<std::int16_t>,   \
    ropufu::aftermath::algebra::fraction<std::int32_t>,   \
    ropufu::aftermath::algebra::fraction<std::int64_t>    \

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_SIGNED_TYPES \
    ropufu::aftermath::algebra::fraction<std::int16_t>,      \
    ropufu::aftermath::algebra::fraction<std::int32_t>,      \
    ropufu::aftermath::algebra::fraction<std::int64_t>       \


TEST_CASE_TEMPLATE("testing zero fractions construction", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() == zero);
    CHECK(tested_t(0) == zero);
    CHECK(tested_t(0, 1) == zero);
    CHECK(tested_t(0, 2) == zero);
    CHECK(tested_t(0, 1729) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions addition", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() + tested_t(0) == zero);
    CHECK(tested_t(0) + tested_t(0, 1) == zero);
    CHECK(tested_t(0, 1) + tested_t(0, 2) == zero);
    CHECK(tested_t(0, 2) + tested_t(0, 1729) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions subtraction", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() - tested_t(0) == zero);
    CHECK(tested_t(0) - tested_t(0, 1) == zero);
    CHECK(tested_t(0, 1) - tested_t(0, 2) == zero);
    CHECK(tested_t(0, 2) - tested_t(0, 1729) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions multiplication 1a", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() * tested_t(1) == zero);
    CHECK(tested_t(0) * tested_t(1) == zero);
    CHECK(tested_t(0, 1) * tested_t(2) == zero);
    CHECK(tested_t(0, 2) * tested_t(3) == zero);
    CHECK(tested_t(0, 1729) * tested_t(5) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions multiplication 1b", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t(1) * tested_t() == zero);
    CHECK(tested_t(1) * tested_t(0) == zero);
    CHECK(tested_t(2) * tested_t(0, 1) == zero);
    CHECK(tested_t(3) * tested_t(0, 2) == zero);
    CHECK(tested_t(5) * tested_t(0, 1729) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions multiplication 2a", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() * 0 == zero);
    CHECK(tested_t(0) * 1 == zero);
    CHECK(tested_t(0, 1) * 2 == zero);
    CHECK(tested_t(0, 2) * 1729 == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions multiplication 2b", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(0 * tested_t() == zero);
    CHECK(1 * tested_t(0) == zero);
    CHECK(2 * tested_t(0, 1) == zero);
    CHECK(1729 * tested_t(0, 2) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions division 1", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() / tested_t(8, 1) == zero);
    CHECK(tested_t(0) / tested_t(13, 1) == zero);
    CHECK(tested_t(0, 1) / tested_t(21, 2) == zero);
    CHECK(tested_t(0, 2) / tested_t(34, 3) == zero);
    CHECK(tested_t(0, 1729) / tested_t(55, 5) == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing zero fractions division 2", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    typename tested_t::integer_type zero {};

    CHECK(tested_t() / 1 == zero);
    CHECK(tested_t(0) / 1 == zero);
    CHECK(tested_t(0, 1) / 2 == zero);
    CHECK(tested_t(0, 2) / 3 == zero);
    CHECK(tested_t(0, 1729) / 5 == zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing general fractions addition 1", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{    
    typename tested_t::integer_type one = 1;

    CHECK(tested_t(1, 3) + tested_t(1, 6) == tested_t(1, 2));
    CHECK(tested_t(1, 2) + tested_t(2, 4) == one);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing signed fractions negation 1", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_SIGNED_TYPES)
{
    tested_t third = tested_t(1, 3);
    tested_t fourth = tested_t(1, 4);
    tested_t sixth = tested_t(1, 6);
    tested_t twelfth = tested_t(1, 12);

    REQUIRE(twelfth - third == -fourth);
    REQUIRE(third - twelfth == fourth);

    fourth.negate();

    REQUIRE(twelfth - fourth == third);
    REQUIRE(third + fourth == twelfth);

    twelfth.negate();

    REQUIRE(twelfth + fourth == -third);
    REQUIRE(fourth - twelfth == -sixth);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing fractions negative float conversion", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_SIGNED_TYPES)
{
    tested_t expected_answer = tested_t(-7, 1729); // approximately -0.004048582995951417.

    tested_t try_below = tested_t(1, 1729);
    tested_t try_above = tested_t(1, 1729);
    ropufu::aftermath::algebra::nearest_fraction(-0.00404, try_below);
    ropufu::aftermath::algebra::nearest_fraction(-0.00405L, try_above);

    CHECK(try_below == expected_answer);
    CHECK(try_above == expected_answer);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing fractions positive float conversion", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    tested_t expected_answer = tested_t(7, 1729); // approximately 0.004048582995951417.

    tested_t try_below = tested_t(1, 1729);
    tested_t try_above = tested_t(1, 1729);
    ropufu::aftermath::algebra::nearest_fraction(0.00404f, try_below);
    ropufu::aftermath::algebra::nearest_fraction(0.00405, try_above);

    CHECK(try_below == expected_answer);
    CHECK(try_above == expected_answer);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing general fractions hash", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    std::hash<tested_t> tested_hash {};
    std::size_t h1 = tested_hash(tested_t(1, 1729));
    std::size_t h2 = tested_hash(tested_t(2, 1729));
    std::size_t h3 = tested_hash(tested_t(1, 3));
    std::size_t h4 = tested_hash(tested_t(2, 3));

    CHECK(h1 != h2);
    CHECK(h1 != h3);
    CHECK(h2 != h4);
    CHECK(h3 != h4);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing general fractions exception handling", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_ALL_TYPES)
{
    CHECK_THROWS_AS({
        tested_t dummy = tested_t(1, 1729);
        dummy /= 0;
    }, const std::logic_error&);
    
    CHECK_THROWS_AS({
        tested_t zero = tested_t(0);
        zero.invert();
    }, const std::logic_error&);

    CHECK_NOTHROW({
        tested_t dummy = tested_t(1, 1729);
        tested_t zero = tested_t(0);
        zero.invert_unchecked();
        zero.invert_unchecked();
        dummy.divide_unchecked(zero);
        zero.set_denominator_unchecked(0);
        zero.divide_unchecked(0);
    });
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_FRACTION_HPP_INCLUDED
