
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

namespace ropufu::tests
{
    template <ropufu::spacing t_spacing_type>
    void make_reference_pair(
        std::vector<typename t_spacing_type::value_type>& raw_value,
        ropufu::aftermath::algebra::interval_based_vector<t_spacing_type>& interval_based_value)
    {
        using value_type = typename t_spacing_type::value_type;
        using interval_type = ropufu::aftermath::algebra::interval<value_type>;

        constexpr std::size_t count = 5;
        constexpr value_type from = 1;
        constexpr value_type to = 5;
        interval_type range(from, to);

        t_spacing_type x {};
        interval_based_value.set_range(range);
        interval_based_value.set_spacing(x);
        interval_based_value.set_count(count);

        if constexpr (t_spacing_type::name == "linear")
        {
            raw_value = { 1, 2, 3, 4, 5 };
        } // if constexpr (...)
        if constexpr (t_spacing_type::name == "logarithmic")
        {
            raw_value = { 1,
                static_cast<value_type>(1.49534878122122),
                static_cast<value_type>(2.23606797749979),
                static_cast<value_type>(3.34370152488211),
                5 };
        } // if constexpr (...)
        if constexpr (t_spacing_type::name == "exponential")
        {
            raw_value = { 1,
                static_cast<value_type>(3.66719608858604),
                static_cast<value_type>(4.32500274735786),
                static_cast<value_type>(4.71840457920730),
                5 };
        } // if constexpr (...)
    } // reference_pair(...)
} // namespace ropufu::tests

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVLA_BASED_VECTOR_SPACING_TYPES \
    ropufu::aftermath::algebra::linear_spacing<std::size_t>,           \
    ropufu::aftermath::algebra::linear_spacing<std::uint16_t>,         \
    ropufu::aftermath::algebra::linear_spacing<std::int16_t>,          \
    ropufu::aftermath::algebra::linear_spacing<std::int32_t>,          \
    ropufu::aftermath::algebra::linear_spacing<std::int64_t>,          \
    ropufu::aftermath::algebra::linear_spacing<float>,                 \
    ropufu::aftermath::algebra::linear_spacing<double>,                \
    ropufu::aftermath::algebra::logarithmic_spacing<std::size_t>,      \
    ropufu::aftermath::algebra::logarithmic_spacing<std::uint16_t>,    \
    ropufu::aftermath::algebra::logarithmic_spacing<std::int16_t>,     \
    ropufu::aftermath::algebra::logarithmic_spacing<std::int32_t>,     \
    ropufu::aftermath::algebra::logarithmic_spacing<std::int64_t>,     \
    ropufu::aftermath::algebra::logarithmic_spacing<float>,            \
    ropufu::aftermath::algebra::logarithmic_spacing<double>,           \
    ropufu::aftermath::algebra::exponential_spacing<std::size_t>,      \
    ropufu::aftermath::algebra::exponential_spacing<std::uint16_t>,    \
    ropufu::aftermath::algebra::exponential_spacing<std::int16_t>,     \
    ropufu::aftermath::algebra::exponential_spacing<std::int32_t>,     \
    ropufu::aftermath::algebra::exponential_spacing<std::int64_t>,     \
    ropufu::aftermath::algebra::exponential_spacing<float>,            \
    ropufu::aftermath::algebra::exponential_spacing<double>            \

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing interval_based_vector json", spacing_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVLA_BASED_VECTOR_SPACING_TYPES)
{
    using value_type = typename spacing_type::value_type;
    using interval_type = ropufu::aftermath::algebra::interval<value_type>;
    using interval_based_type = ropufu::aftermath::algebra::interval_based_vector<spacing_type>;

    interval_based_type x {};
    interval_based_type y {};
    interval_based_type z {};

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

TEST_CASE_TEMPLATE("testing interval_based_vector explosion", spacing_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVLA_BASED_VECTOR_SPACING_TYPES)
{
    using value_type = typename spacing_type::value_type;
    using interval_based_type = ropufu::aftermath::algebra::interval_based_vector<spacing_type>;

    interval_based_type x {};
    std::vector<value_type> reference_seq {};
    std::vector<value_type> exploded_seq {};

    ropufu::tests::template make_reference_pair<spacing_type>(reference_seq, x);
    x.explode(exploded_seq);

    REQUIRE_EQ(exploded_seq.size(), reference_seq.size());
    
    using distance_type = double;
    constexpr distance_type tolerance = 1e-5;
    distance_type error = ropufu::tests::vector_distance<distance_type>(reference_seq, exploded_seq);
    CHECK_LT(error, tolerance);
} // TEST_CASE_TEMPLATE(...)

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing vector_initializer_t serialization", spacing_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVLA_BASED_VECTOR_SPACING_TYPES)
{
    using value_type = typename spacing_type::value_type;
    using interval_based_type = ropufu::aftermath::algebra::interval_based_vector<spacing_type>;
    using initializer_type = ropufu::vector_initializer_t<
        ropufu::aftermath::algebra::linear_spacing<value_type>,
        ropufu::aftermath::algebra::logarithmic_spacing<value_type>,
        ropufu::aftermath::algebra::exponential_spacing<value_type>>;

    interval_based_type x {};
    std::vector<value_type> reference_seq {};

    ropufu::tests::template make_reference_pair<spacing_type>(reference_seq, x);

    initializer_type explicit_init {};
    initializer_type range_based_init = x;
    REQUIRE_NE(range_based_init.index(), 0);

    nlohmann::json j_explicit;
    nlohmann::json j_range_based;
    REQUIRE(ropufu::try_serialize(j_explicit, reference_seq, explicit_init));
    REQUIRE(ropufu::try_serialize(j_range_based, reference_seq, range_based_init));
    CHECK_NE(j_explicit, j_range_based);
} // TEST_CASE_TEMPLATE(...)
#endif

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing vector_initializer_t deserialization", spacing_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVLA_BASED_VECTOR_SPACING_TYPES)
{
    using value_type = typename spacing_type::value_type;
    using interval_based_type = ropufu::aftermath::algebra::interval_based_vector<spacing_type>;
    using spacing_a_type = ropufu::aftermath::algebra::linear_spacing<value_type>;
    using spacing_b_type = ropufu::aftermath::algebra::logarithmic_spacing<value_type>;
    using spacing_c_type = ropufu::aftermath::algebra::exponential_spacing<value_type>;

    interval_based_type x {};
    std::vector<value_type> reference_seq {};

    ropufu::tests::template make_reference_pair<spacing_type>(reference_seq, x);

    nlohmann::json j_explicit = reference_seq;
    nlohmann::json j_range_based = x;

    std::vector<value_type> from_explicit;
    std::vector<value_type> from_range_based;
    REQUIRE(ropufu::template try_deserialize<spacing_a_type, spacing_b_type, spacing_c_type>(j_explicit, from_explicit));
    REQUIRE(ropufu::template try_deserialize<spacing_a_type, spacing_b_type, spacing_c_type>(j_range_based, from_range_based));
    REQUIRE_EQ(from_explicit.size(), reference_seq.size());
    REQUIRE_EQ(from_range_based.size(), reference_seq.size());
    
    using distance_type = double;
    constexpr distance_type tolerance = 1e-5;
    distance_type error_explicit = ropufu::tests::vector_distance<distance_type>(reference_seq, from_explicit);
    distance_type error_range_based = ropufu::tests::vector_distance<distance_type>(reference_seq, from_range_based);
    CHECK_LT(error_explicit, tolerance);
    CHECK_LT(error_range_based, tolerance);
} // TEST_CASE_TEMPLATE(...)
#endif

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED
