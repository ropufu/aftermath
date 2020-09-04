
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_NOEXCEPT_JSON_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_NOEXCEPT_JSON_HPP_INCLUDED

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include "../../ropufu/noexcept_json.hpp"
#include "../core.hpp"

#include <cstdint>  // std::int??_t, std::uint??_t
#include <map>      // std::map
#include <optional> // std::optional
#include <set>      // std::set
#include <string>   // std::string
#include <variant>  // std::variant, std::holds_alternative, std::get
#include <vector>   // std::vector

TEST_CASE("noexcept json std::variant unsigned/signed: unsigned")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("17", j));

    using a_type = std::uint32_t;
    using b_type = std::int32_t;
    std::variant<a_type, b_type> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(std::holds_alternative<a_type>(x));
    a_type a = std::get<a_type>(x);
    CHECK_EQ(a, 17);
} // TEST_CASE(...)

TEST_CASE("noexcept json std::variant unsigned/signed: signed")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("-29", j));

    using a_type = std::uint32_t;
    using b_type = std::int32_t;
    std::variant<a_type, b_type> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(std::holds_alternative<b_type>(x));
    b_type b = std::get<b_type>(x);
    CHECK_EQ(b, -29);
} // TEST_CASE(...)

TEST_CASE("noexcept json std::variant vector/map: vector")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("[1, 2, 3]", j));

    using a_type = std::vector<std::size_t>;
    using b_type = std::map<std::string, std::size_t>;
    std::variant<a_type, b_type> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(std::holds_alternative<a_type>(x));
    a_type a = std::get<a_type>(x);
    CHECK_EQ(a.size(), 3);
} // TEST_CASE(...)

TEST_CASE("noexcept json std::variant vector/map: map")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("{\"ropufu\": 1729}", j));

    using a_type = std::vector<std::size_t>;
    using b_type = std::map<std::string, std::size_t>;
    std::variant<a_type, b_type> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(std::holds_alternative<b_type>(x));
    b_type b = std::get<b_type>(x);
    CHECK_EQ(b["ropufu"], 1729);
} // TEST_CASE(...)

TEST_CASE("noexcept json std::optional null")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("1729", j));

    std::optional<std::int32_t> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(x.has_value());
    CHECK_EQ(x.value(), 1729);
} // TEST_CASE(...)

TEST_CASE("noexcept json std::optional not null")
{
    nlohmann::json j {};
    REQUIRE(ropufu::noexcept_json::try_parse("null", j));

    std::optional<std::int32_t> x {};
    REQUIRE(ropufu::noexcept_json::try_get(j, x));

    REQUIRE(!x.has_value());
} // TEST_CASE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_NOEXCEPT_JSON_HPP_INCLUDED
