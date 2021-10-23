
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_D_HPP_INCLUDED
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_D_HPP_INCLUDED
#ifndef ROPUFU_NO_JSON

#if __has_include("test_d.g.hpp")
#include "test_d.g.hpp"
#include <iostream>
namespace ropufu::tests::json_schema_to_hpp
{
    bool test_d::hello_world() const noexcept { return true; }
} // namespace ropufu::tests::json_schema_to_hpp
#else

#include <concepts>
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>

#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <ostream>     // std::ostream
#include <string>      // std::string
#include <string_view> // std::string_view

namespace ropufu::tests::json_schema_to_hpp
{
    struct test_d;

    void to_json(nlohmann::json& j, const test_d& x) noexcept;
    void from_json(const nlohmann::json& j, test_d& x);

    struct test_d
    {
        using type = test_d;
        static const test_d a;
        static const test_d b;
        static const test_d c;

        constexpr test_d(const char*) noexcept;
        constexpr test_d(std::string_view) noexcept;
        constexpr test_d(const std::string&) noexcept;

        constexpr std::string_view to_string_view() const noexcept;

        bool operator ==(const type& other) const noexcept;
        bool operator !=(const type& other) const noexcept;

        friend std::ostream& operator <<(std::ostream& os, type) { return os; }
        friend void to_json(nlohmann::json&, const type&) noexcept { }
        friend void from_json(const nlohmann::json&, type&) { }

        bool hello_world() const noexcept;
    }; // struct test_d
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_d>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_d;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept;
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <>
    struct hash<ropufu::tests::json_schema_to_hpp::test_d>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_d;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept;
    }; // struct hash<...>
} // namespace std

#endif // __has_include("test_d.g.hpp")
#endif // ROPUFU_NO_JSON
#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_D_HPP_INCLUDED
