
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_HPP_INCLUDED
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_HPP_INCLUDED
#ifndef ROPUFU_NO_JSON

#if __has_include("test_a.g.hpp")
#include "test_a.g.hpp"
#else

#include <concepts>
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <string>     // std::string

namespace ropufu::tests::json_schema_to_hpp
{
    struct test_a;

    void to_json(nlohmann::json& j, const test_a& x) noexcept;
    void from_json(const nlohmann::json& j, test_a& x);

    struct test_a
    {
        using type = test_a;
        using age_type = int;
        using label_type = std::string;

        constexpr void validate() const noexcept;

        test_a() noexcept;
        explicit test_a(age_type age) noexcept;

        age_type age() const noexcept;
        void set_age(age_type value) noexcept;

        const label_type& label() const noexcept;
        void set_label(const label_type& value) noexcept;

        bool operator ==(const type& other) const noexcept;
        bool operator !=(const type& other) const noexcept;

        friend void to_json(nlohmann::json&, const type&) noexcept { }
        friend void from_json(const nlohmann::json&, type&) { }
    }; // struct test_a
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_a>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_a;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept;
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <>
    struct hash<ropufu::tests::json_schema_to_hpp::test_a>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_a;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept;
    }; // struct hash<...>
} // namespace std

#endif // __has_include("test_a.g.hpp")
#endif // ROPUFU_NO_JSON
#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_HPP_INCLUDED
