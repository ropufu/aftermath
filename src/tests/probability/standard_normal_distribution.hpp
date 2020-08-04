
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/probability/standard_normal_distribution.hpp"

#include <array>      // std::array
#include <cmath>      // std::abs
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_ALL_TYPES        \
    ropufu::aftermath::probability::standard_normal_distribution<float, float, float>,   \
    ropufu::aftermath::probability::standard_normal_distribution<float, float, double>,  \
    ropufu::aftermath::probability::standard_normal_distribution<float, double, double>, \
    ropufu::aftermath::probability::standard_normal_distribution<double, double, double> \

TEST_CASE_TEMPLATE("testing standard_normal_distribution cdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::probability_type;

    std::array<argument_type, 7> arguments { -3, -2, -1, 0, 1, 2, 3 };
    std::array<answer_type, 7> answers {
        answer_type(0.0013498980316301),
        answer_type(0.0227501319481792),
        answer_type(0.1586552539314570),
        answer_type(0.5),
        answer_type(0.8413447460685429),
        answer_type(0.9772498680518208),
        answer_type(0.9986501019683699) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.cdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_normal_distribution pdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::expectation_type;

    std::array<argument_type, 7> arguments { -3, -2, -1, 0, 1, 2, 3 };
    std::array<answer_type, 7> answers {
        answer_type(0.004431848411938),
        answer_type(0.053990966513188),
        answer_type(0.241970724519143),
        answer_type(0.398942280401433),
        answer_type(0.241970724519143),
        answer_type(0.053990966513188),
        answer_type(0.004431848411938) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.pdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_normal_distribution quantiles", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_ALL_TYPES)
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
        answer_type(-2.326347874040841),
        answer_type(-1.644853626951473),
        answer_type(-1.281551565544601),
        answer_type(-0.524400512708041),
        answer_type(0),
        answer_type(0.253347103135800),
        answer_type(0.841621233572914) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested {};
    CHECK(std::abs(tested.numerical_quantile(levels[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing standard_normal_distribution exception handling", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::probability_type;
    tested_t tested {};

    CHECK_NOTHROW(tested.numerical_quantile(0));
    CHECK_NOTHROW(tested.numerical_quantile(1));
    CHECK_THROWS_AS(tested.numerical_quantile(-1), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(2), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(std::numeric_limits<argument_type>::infinity()), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(std::numeric_limits<argument_type>::quiet_NaN()), const std::logic_error&);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED
