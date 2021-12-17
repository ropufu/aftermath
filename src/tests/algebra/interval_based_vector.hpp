
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED

#include <doctest/doctest.h>
#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

#include "../core.hpp"
#include "../../ropufu/algebra/interval.hpp"
#include "../../ropufu/algebra/interval_based_vector.hpp"
#include "../../ropufu/algebra/interval_spacing.hpp"
#include "../../ropufu/noexcept_json.hpp"
#include "../../ropufu/vector_extender.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int16_t, std::int32_t, std::int64_t
#include <functional> // std::hash
#include <map>        // std::map
#include <string>     // std::string
#include <vector>     // std::vector

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_FLOAT_TYPES    \
    ropufu::aftermath::algebra::interval<float>,            \
    ropufu::aftermath::algebra::interval<double>            \

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES      \
    ropufu::aftermath::algebra::interval<std::size_t>,      \
    ropufu::aftermath::algebra::interval<std::uint16_t>,    \
    ropufu::aftermath::algebra::interval<std::int16_t>,     \
    ropufu::aftermath::algebra::interval<std::int32_t>,     \
    ropufu::aftermath::algebra::interval<std::int64_t>,     \
    ropufu::aftermath::algebra::interval<float>,            \
    ropufu::aftermath::algebra::interval<double>            \

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing interval_based_vector json", interval_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    using value_type = typename interval_type::value_type;

    using spacing_a_type = ropufu::aftermath::algebra::linear_spacing<value_type>;
    using spacing_b_type = ropufu::aftermath::algebra::logarithmic_spacing<value_type>;
    using spacing_c_type = ropufu::aftermath::algebra::exponential_spacing<value_type>;

    using interval_based_a_type = ropufu::aftermath::algebra::interval_based_vector<spacing_a_type>;
    using interval_based_b_type = ropufu::aftermath::algebra::interval_based_vector<spacing_b_type>;
    using interval_based_c_type = ropufu::aftermath::algebra::interval_based_vector<spacing_c_type>;

    interval_based_a_type x {};
    interval_based_b_type y {};
    interval_based_c_type z {};

    interval_type a {1, 1729};
    interval_type b {2, 13};
    interval_type c {27, 27};

    x.set_range(a);
    y.set_range(b);
    z.set_range(c);

    x.set_count(1);
    y.set_count(7);
    z.set_count(29);

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(x, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(y, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(z, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing interval_based_vector explosion", interval_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    using value_type = typename interval_type::value_type;

    using spacing_a_type = ropufu::aftermath::algebra::linear_spacing<value_type>;
    using spacing_b_type = ropufu::aftermath::algebra::logarithmic_spacing<value_type>;
    using spacing_c_type = ropufu::aftermath::algebra::exponential_spacing<value_type>;

    using interval_based_a_type = ropufu::aftermath::algebra::interval_based_vector<spacing_a_type>;
    using interval_based_b_type = ropufu::aftermath::algebra::interval_based_vector<spacing_b_type>;
    using interval_based_c_type = ropufu::aftermath::algebra::interval_based_vector<spacing_c_type>;

    interval_based_a_type x {};
    interval_based_b_type y {};
    interval_based_c_type z {};

    constexpr std::size_t n = 5;
    constexpr value_type from = 1;
    constexpr value_type to = 5;

    x.set_range({from, to});
    y.set_range({from, to});
    z.set_range({from, to});

    x.set_count(n);
    y.set_count(n);
    z.set_count(n);

    std::vector<value_type> linear_seq {};
    std::vector<value_type> logarithmic_seq {};
    std::vector<value_type> exponential_seq {};

    x.explode(linear_seq);
    y.explode(logarithmic_seq);
    z.explode(exponential_seq);

    REQUIRE(linear_seq.size() == n);
    REQUIRE(logarithmic_seq.size() == n);
    REQUIRE(exponential_seq.size() == n);

    // Linear spacing is easy: it all happens in one space with no intermediate transformations.
    std::vector<value_type> expected_lin_seq = { 1, 2, 3, 4, 5 };
    for (std::size_t i = 0; i < n; ++i) CHECK(linear_seq[i] == expected_lin_seq[i]);

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
        error_log_seq[i] = (logarithmic_seq[i] > expected_log_seq[i]) ? (logarithmic_seq[i] - expected_log_seq[i]) : (expected_log_seq[i] - logarithmic_seq[i]);
        error_exp_seq[i] = (exponential_seq[i] > expected_exp_seq[i]) ? (exponential_seq[i] - expected_exp_seq[i]) : (expected_exp_seq[i] - exponential_seq[i]);
    } // for (...)
    for (std::size_t i = 0; i < n; ++i) CHECK(error_log_seq[i] < tolerance);
    for (std::size_t i = 0; i < n; ++i) CHECK(error_exp_seq[i] < tolerance);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED
