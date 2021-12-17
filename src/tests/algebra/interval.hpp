
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED

#include <doctest/doctest.h>
#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

#include "../core.hpp"
#include "../../ropufu/algebra/interval.hpp"
#include "../../ropufu/noexcept_json.hpp"

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
TEST_CASE_TEMPLATE("testing interval json", interval_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    interval_type a {1, 1729};
    interval_type b {2, 13};
    interval_type c {27, 27};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(b, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(c, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing interval noexcept json", interval_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    interval_type a {1, 1729};
    interval_type b {2, 13};
    interval_type c {27, 27};

    nlohmann::json j = {
        {"a", a},
        {"b", b},
        {"gamma", c}
    };

    std::map<std::string, interval_type> m {};

    REQUIRE(ropufu::noexcept_json::try_get(j, m));

    CHECK_EQ(m["a"], a);
    CHECK_EQ(m["b"], b);
    CHECK_EQ(m["gamma"], c);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing interval hash", interval_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_ALL_TYPES)
{
    std::hash<interval_type> tested_hash {};
    std::size_t h1 = tested_hash(interval_type(1, 1729));
    std::size_t h2 = tested_hash(interval_type(2, 1729));
    std::size_t h3 = tested_hash(interval_type(1, 3));
    std::size_t h4 = tested_hash(interval_type(2, 3));

    CHECK(h1 != h2);
    CHECK(h1 != h3);
    CHECK(h2 != h4);
    CHECK(h3 != h4);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_RANGE_HPP_INCLUDED
