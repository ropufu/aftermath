
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_HPP_INCLUDED
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_HPP_INCLUDED
#ifndef ROPUFU_NO_JSON

#if __has_include("test_c.g.hpp")
#include "test_c.g.hpp"
#else

#include "test_a.hpp"
#include "test_b.hpp"

#include <concepts>
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>

#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <string>     // std::string

namespace ropufu::tests::json_schema_to_hpp::detail
{
    template <typename t_value_type>
    struct pair;

    template <typename t_value_type>
    void to_json(nlohmann::json& j, const pair<t_value_type>& x) noexcept;
    template <typename t_value_type>
    void from_json(const nlohmann::json& j, pair<t_value_type>& x);

    template <typename t_value_type>
    struct pair
    {
        using type = pair<t_value_type>;
        using value_type = t_value_type;
        using second_type = value_type;
        using first_type = value_type;

        constexpr void validate() const noexcept;

        pair() noexcept;
        pair(second_type second, first_type first) noexcept;

        second_type second() const noexcept;
        void set_second(second_type value) noexcept;

        first_type first() const noexcept;
        void set_first(first_type value) noexcept;

        bool operator ==(const type& other) const noexcept;
        bool operator !=(const type& other) const noexcept;

        friend void to_json(nlohmann::json&, const type&) noexcept { }
        friend void from_json(const nlohmann::json&, type&) { }
    }; // struct pair
} // namespace ropufu::tests::json_schema_to_hpp::detail

namespace ropufu
{
    template <typename t_value_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept;
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <typename t_value_type>
    struct hash<ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept;
    }; // struct hash<...>
} // namespace std

namespace ropufu::tests::json_schema_to_hpp
{
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct test_c;

    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    void to_json(nlohmann::json& j, const test_c<t_integer_type, t_probability_type>& x) noexcept;
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    void from_json(const nlohmann::json& j, test_c<t_integer_type, t_probability_type>& x);

    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct test_c
        : public json_schema_to_hpp::test_a,
        public json_schema_to_hpp::test_b<t_integer_type>
    {
        using type = test_c<t_integer_type, t_probability_type>;
        using integer_type = t_integer_type;
        using probability_type = t_probability_type;
        using base_type_0 = json_schema_to_hpp::test_a;
        using base_type_1 = json_schema_to_hpp::test_b<t_integer_type>;
        using age_type = integer_type;
        using names_type = std::set<std::string>;
        using tag_type = json_schema_to_hpp::test_a;
        using chances_type = detail::pair<probability_type>;

        void validate() const;

        test_c();

        const chances_type& chances() const noexcept;
        void set_chances(const chances_type& value) noexcept;

        bool operator ==(const type& other) const noexcept;
        bool operator !=(const type& other) const noexcept;

        friend void to_json(nlohmann::json&, const type&) noexcept { }
        friend void from_json(const nlohmann::json&, type&) { }
    }; // struct test_c
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept;
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct hash<ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept;
    }; // struct hash<...>
} // namespace std

#endif // __has_include("test_c.g.hpp")
#endif // ROPUFU_NO_JSON
#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_HPP_INCLUDED
