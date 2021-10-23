
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_HPP_INCLUDED
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_HPP_INCLUDED
#ifndef ROPUFU_NO_JSON

#if __has_include("test_b.g.hpp")
#include "test_b.g.hpp"
#else

#include "test_a.hpp"

#include <concepts>
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <string>     // std::string

namespace ropufu::tests::json_schema_to_hpp
{
    template <ropufu::integer t_integer_type>
    struct test_b;

    template <ropufu::integer t_integer_type>
    void to_json(nlohmann::json& j, const test_b<t_integer_type>& x) noexcept;
    template <ropufu::integer t_integer_type>
    void from_json(const nlohmann::json& j, test_b<t_integer_type>& x);

    template <ropufu::integer t_integer_type>
    struct test_b
    {
        using type = test_b<t_integer_type>;
        using integer_type = t_integer_type;
        using names_type = std::set<std::string>;
        using tag_type = json_schema_to_hpp::test_a;

        void validate() const;

        test_b();

        explicit test_b(names_type names);

        const names_type& names() const noexcept;
        void set_names(const names_type& value);

        const tag_type& tag() const noexcept;
        void set_tag(const tag_type& value) noexcept;

        bool operator ==(const type& other) const noexcept;
        bool operator !=(const type& other) const noexcept;

        friend void to_json(nlohmann::json&, const type&) noexcept { }
        friend void from_json(const nlohmann::json&, type&) { }
    }; // struct test_b
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <ropufu::integer t_integer_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept;
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <ropufu::integer t_integer_type>
    struct hash<ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept;
    }; // struct hash<...>
} // namespace std

#endif // __has_include("test_b.g.hpp")
#endif // ROPUFU_NO_JSON
#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_HPP_INCLUDED
