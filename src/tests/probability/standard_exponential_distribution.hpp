
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/probability/standard_exponential_distribution.hpp"

#include <array>      // std::array
#include <cmath>      // std::abs
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_ALL_TYPES        \
    ropufu::aftermath::probability::standard_exponential_distribution<float, float, float>,   \
    ropufu::aftermath::probability::standard_exponential_distribution<float, float, double>,  \
    ropufu::aftermath::probability::standard_exponential_distribution<float, double, double>, \
    ropufu::aftermath::probability::standard_exponential_distribution<double, double, double> \

TEST_CASE_TEMPLATE("testing standard_exponential_distribution cdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::probability_type;

    std::array<argument_type, 5> arguments { -1, 0, 1, 2, 3 };
    std::array<answer_type, 5> answers {
        answer_type(0),
        answer_type(0),
        answer_type(0.63212055882856),
        answer_type(0.86466471676339),
        answer_type(0.95021293163214) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.cdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[4]) - answers[4]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_exponential_distribution pdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::expectation_type;

    std::array<argument_type, 5> arguments { -1, 0, 1, 2, 3 };
    std::array<answer_type, 5> answers {
        answer_type(0),
        answer_type(1),
        answer_type(0.367879441171442),
        answer_type(0.135335283236613),
        answer_type(0.049787068367864) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.pdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[4]) - answers[4]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_exponential_distribution quantiles", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::probability_type;
    using answer_type = typename tested_t::expectation_type;

    std::array<argument_type, 7> levels {
        argument_type(0.01),
        argument_type(0.05),
        argument_type(0.10),
        argument_type(0.30),
        argument_type(0.50),
        argument_type(0.60),
        argument_type(0.80) };
    std::array<answer_type, 7> answers {
        answer_type(0.010050335853501),
        answer_type(0.051293294387551),
        answer_type(0.105360515657826),
        answer_type(0.356674943938732),
        answer_type(0.693147180559945),
        answer_type(0.916290731874155),
        answer_type(1.609437912434101) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.quantile(levels[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_exponential_distribution exception handling", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::probability_type;
    tested_t tested {};

    CHECK_NOTHROW(tested.quantile(0));
    CHECK_NOTHROW(tested.quantile(1));
    CHECK_THROWS_AS(tested.quantile(-1), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(2), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(std::numeric_limits<argument_type>::infinity()), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(std::numeric_limits<argument_type>::quiet_NaN()), const std::logic_error&);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
