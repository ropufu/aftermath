
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/range.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int16_t, std::int32_t, std::int64_t
#include <functional> // std::hash
#include <vector>     // std::vector

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_XLL_TYPES   \
    ropufu::aftermath::algebra::range<float>,            \
    ropufu::aftermath::algebra::range<double>            \

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES   \
    ropufu::aftermath::algebra::range<std::size_t>,      \
    ropufu::aftermath::algebra::range<std::uint16_t>,    \
    ropufu::aftermath::algebra::range<std::int16_t>,     \
    ropufu::aftermath::algebra::range<std::int32_t>,     \
    ropufu::aftermath::algebra::range<std::int64_t>,     \
    ropufu::aftermath::algebra::range<float>,            \
    ropufu::aftermath::algebra::range<double>            \


TEST_CASE_TEMPLATE("testing range json", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    tested_t a = tested_t(1, 1729);
    tested_t b = tested_t(13, 2);
    tested_t c = tested_t(27, 27);

    CHECK(ropufu::aftermath::tests::does_json_round_trip(a));
    CHECK(ropufu::aftermath::tests::does_json_round_trip(b));
    CHECK(ropufu::aftermath::tests::does_json_round_trip(c));
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing range explosion", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    using value_type = typename tested_t::value_type;
    constexpr std::size_t n = 5;
    value_type from = 1;
    value_type to = 5;

    tested_t a = tested_t(from, to);
    std::vector<value_type> a_lin_seq {};
    std::vector<value_type> a_log_seq {};
    std::vector<value_type> a_exp_seq {};

    a.explode(a_lin_seq, n);
    a.explode(a_log_seq, n, ropufu::aftermath::algebra::logarithmic_spacing<value_type>());
    a.explode(a_exp_seq, n, ropufu::aftermath::algebra::exponential_spacing<value_type>());

    REQUIRE(a_lin_seq.size() == n);
    REQUIRE(a_log_seq.size() == n);
    REQUIRE(a_exp_seq.size() == n);

    // Linear spacing is easy: it all happens in one space with no intermediate transformations.
    std::vector<value_type> expected_lin_seq = { 1, 2, 3, 4, 5 };
    for (std::size_t i = 0; i < n; ++i) CHECK(a_lin_seq[i] == expected_lin_seq[i]);

    // Non-linear spacing is where things get a little more complicated.
    constexpr double tolerance = 1e-5;
    std::vector<double> expected_log_seq_x = { 1, 1.49534878122122, 2.23606797749979, 3.34370152488211, 5 };
    std::vector<double> expected_exp_seq_x = { 1, 3.66719608858604, 4.32500274735786, 4.71840457920730, 5 };

    std::vector<value_type> expected_log_seq(n);
    std::vector<value_type> expected_exp_seq(n);
    for (std::size_t i = 0; i < n; ++i)
    {
        expected_log_seq[i] = static_cast<value_type>(expected_log_seq_x[i]);
        expected_exp_seq[i] = static_cast<value_type>(expected_exp_seq_x[i]);
    } // for (...)

    std::vector<value_type> error_log_seq(n);
    std::vector<value_type> error_exp_seq(n);
    // Make sure unsigned types are handled properly.
    for (std::size_t i = 0; i < n; ++i)
    {
        error_log_seq[i] = (a_log_seq[i] > expected_log_seq[i]) ? (a_log_seq[i] - expected_log_seq[i]) : (expected_log_seq[i] - a_log_seq[i]);
        error_exp_seq[i] = (a_exp_seq[i] > expected_exp_seq[i]) ? (a_exp_seq[i] - expected_exp_seq[i]) : (expected_exp_seq[i] - a_exp_seq[i]);
    } // for (...)
    for (std::size_t i = 0; i < n; ++i) CHECK(error_log_seq[i] < tolerance);
    for (std::size_t i = 0; i < n; ++i) CHECK(error_exp_seq[i] < tolerance);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing range hash", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
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

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED
