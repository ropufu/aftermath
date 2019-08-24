
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/probability/exponential_distribution.hpp"

#include <array>     // std::array
#include <cmath>     // std::abs
#include <cstddef>   // std::size_t
#include <limits>    // std::numeric_limits
#include <stdexcept> // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_ALL_TYPES        \
    ropufu::aftermath::probability::exponential_distribution<float, float, float>,   \
    ropufu::aftermath::probability::exponential_distribution<float, float, double>,  \
    ropufu::aftermath::probability::exponential_distribution<float, double, double>, \
    ropufu::aftermath::probability::exponential_distribution<double, double, double> \

TEST_CASE_TEMPLATE("testing exponential_distribution cdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::probability_type;

    std::array<argument_type, 5> arguments { -1, 0, 1, 2, 3 };
    std::array<answer_type, 5> answers {
        answer_type(0),
        answer_type(0),
        answer_type(0.95021293163214),
        answer_type(0.99752124782333),
        answer_type(0.99987659019591) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { 3 };
    CHECK(std::abs(tested.cdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.cdf(arguments[4]) - answers[4]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing exponential_distribution pdf", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::value_type;
    using answer_type = typename tested_t::expectation_type;

    std::array<argument_type, 5> arguments { -1, 0, 1, 2, 3 };
    std::array<answer_type, 5> answers {
        answer_type(0),
        answer_type(3),
        answer_type(0.14936120510359185),
        answer_type(0.00743625652999908),
        answer_type(0.00037022941226004) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { 3 };
    CHECK(std::abs(tested.pdf(arguments[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.pdf(arguments[4]) - answers[4]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing exponential_distribution quantiles", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
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
        answer_type(0.0033501119511671),
        answer_type(0.0170977647958502),
        answer_type(0.0351201718859421),
        answer_type(0.1188916479795774),
        answer_type(0.2310490601866484),
        answer_type(0.3054302439580516),
        answer_type(0.5364793041447001) };
    answer_type tolerance = answer_type(1e-6);

    tested_t tested { 3 };
    CHECK(std::abs(tested.quantile(levels[0]) - answers[0]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[1]) - answers[1]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[2]) - answers[2]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[3]) - answers[3]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[4]) - answers[4]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[5]) - answers[5]) < tolerance);
    CHECK(std::abs(tested.quantile(levels[6]) - answers[6]) < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing exponential_distribution exception handling", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_ALL_TYPES)
{
    using argument_type = typename tested_t::probability_type;
    tested_t tested { 3 };

    CHECK_NOTHROW(tested.quantile(0));
    CHECK_NOTHROW(tested.quantile(1));
    CHECK_THROWS_AS(tested.quantile(-1), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(2), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(std::numeric_limits<argument_type>::infinity()), const std::logic_error&);
    CHECK_THROWS_AS(tested.quantile(std::numeric_limits<argument_type>::quiet_NaN()), const std::logic_error&);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
