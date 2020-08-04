
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/probability/normal_distribution.hpp"

#include <array>     // std::array
#include <cmath>     // std::abs
#include <cstddef>   // std::size_t
#include <limits>    // std::numeric_limits
#include <stdexcept> // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_ALL_TYPES        \
    ropufu::aftermath::probability::normal_distribution<float, float, float>,   \
    ropufu::aftermath::probability::normal_distribution<float, float, double>,  \
    ropufu::aftermath::probability::normal_distribution<float, double, double>, \
    ropufu::aftermath::probability::normal_distribution<double, double, double> \

TEST_CASE_TEMPLATE("testing normal_distribution cdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::probability_type;

    std::array<argument_type, 7> arguments { -3, -2, -1, 0, 1, 2, 3 };
    std::array<answer_type, 7> answers {
        answer_type(0.38754848109799),
        answer_type(0.44320150318353),
        answer_type(0.5),
        answer_type(0.55679849681647),
        answer_type(0.61245151890201),
        answer_type(0.66588242910238),
        answer_type(0.71614541690132) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { -1, 7 };
    CHECK(std::abs(tested.cdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing normal_distribution pdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::expectation_type;

    std::array<argument_type, 7> arguments { -3, -2, -1, 0, 1, 2, 3 };
    std::array<answer_type, 7> answers {
        answer_type(0.054712394277745),
        answer_type(0.056413162847180),
        answer_type(0.056991754343062),
        answer_type(0.056413162847180),
        answer_type(0.054712394277745),
        answer_type(0.051990960245069),
        answer_type(0.048406847965255) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { -1, 7 };
    CHECK(std::abs(tested.pdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing normal_distribution quantiles", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_ALL_TYPES)
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
        answer_type(-17.2844351182859),
        answer_type(-12.5139753886603),
        answer_type(-9.9708609588122),
        answer_type(-4.6708035889563),
        answer_type(-1),
        answer_type(0.7734297219506),
        answer_type(4.8913486350104) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { -1, 7 };
    CHECK(std::abs(tested.numerical_quantile(levels[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.numerical_quantile(levels[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing normal_distribution exception handling", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::probability_type;
    tested_t tested { -1, 7 };

    CHECK_NOTHROW(tested.numerical_quantile(0));
    CHECK_NOTHROW(tested.numerical_quantile(1));
    CHECK_THROWS_AS(tested.numerical_quantile(-1), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(2), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(std::numeric_limits<argument_type>::infinity()), const std::logic_error&);
    CHECK_THROWS_AS(tested.numerical_quantile(std::numeric_limits<argument_type>::quiet_NaN()), const std::logic_error&);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
