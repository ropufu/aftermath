
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_83C6F7C946734848BB292B2949FAD1E9
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_83C6F7C946734848BB292B2949FAD1E9

//=======================================================================
// This file was generated automatically.
// Please do not edit it directly, since any changes may be overwritten.
//=======================================================================
// UTC Date: 2/13/2021 7:55:15 PM
//=======================================================================

#include "test_a.g.hpp"
#include "test_b.g.hpp"
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>
#include <ropufu/number_traits.hpp>

#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

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

        // ~~ Field typedefs ~~
        using second_type = value_type;
        using first_type = value_type;

        // ~~ Json keys ~~
        static constexpr std::string_view jstr_second = "second";
        static constexpr std::string_view jstr_first = "first";

        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    protected:
        second_type m_second = {};
        first_type m_first = {};

    public:
        constexpr void validate() const noexcept { }

        pair() noexcept { }

        pair(second_type second, first_type first) noexcept
            : m_second(second),
            m_first(first)
        { }

        second_type second() const noexcept { return this->m_second; }

        void set_second(second_type value) noexcept { this->m_second = value; }

        first_type first() const noexcept { return this->m_first; }

        void set_first(first_type value) noexcept { this->m_first = value; }

        /** Checks if this object is equivalent to \param other. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_second == other.m_second &&
                this->m_first == other.m_first;
        } // operator ==(...)

        /** Checks if this object is not equivalent to \param other. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_second, x.m_second},
                {type::jstr_first, x.m_first}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, pair<t_value_type>& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <pair> failed: " + j.dump());
        } // from_json(...)
    }; // struct pair
} // namespace ropufu::tests::json_schema_to_hpp::detail

namespace ropufu
{
    template <typename t_value_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!noexcept_json::required(j, result_type::jstr_second, x.m_second)) return false;
            if (!noexcept_json::required(j, result_type::jstr_first, x.m_first)) return false;

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <typename t_value_type>
    struct hash<ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::detail::pair<t_value_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / 2;
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::second_type> second_hasher = {};
            std::hash<typename argument_type::first_type> first_hasher = {};

            result ^= (second_hasher(x.m_second) << ((shift * 0) % total_width));
            result ^= (first_hasher(x.m_first) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
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

        // ~~ Field typedefs ~~
        using age_type = integer_type;
        using names_type = std::set<std::string>;
        using tag_type = json_schema_to_hpp::test_a;
        using chances_type = detail::pair<probability_type>;

        // ~~ Json keys ~~
        static constexpr std::string_view jstr_age = "age";
        static constexpr std::string_view jstr_names = "names";
        static constexpr std::string_view jstr_tag = "tag";
        static constexpr std::string_view jstr_chances = "chances";

        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    protected:
        chances_type m_chances = detail::pair<probability_type>{0.5, 0.5};

        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            const type& self = *this;
            if (self.m_names.size() < 1) return "There must be at least one name.";
            for (const auto& x : self.m_names)
            {
                if (x.size() < 1) return "Name cannot be empty.";
            } // for (...)
            return std::nullopt;
        } // error_message(...)

    public:
        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

        test_c() { this->validate(); } // Will throw if default values do not pass validation.

        const chances_type& chances() const noexcept { return this->m_chances; }

        void set_chances(const chances_type& value) noexcept { this->m_chances = value; }

        /** Checks if this object is equivalent to \param other. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_age == other.m_age &&
                this->m_names == other.m_names &&
                this->m_tag == other.m_tag &&
                this->m_chances == other.m_chances;
        } // operator ==(...)

        /** Checks if this object is not equivalent to \param other. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{};

            static type default_instance {};

            if (x.m_age != default_instance.m_age) j[std::string(type::jstr_age)] = x.m_age;
            if (!(x.m_names.empty())) j[std::string(type::jstr_names)] = x.m_names;
            if (x.m_tag != default_instance.m_tag) j[std::string(type::jstr_tag)] = x.m_tag;
            if (x.m_chances != default_instance.m_chances) j[std::string(type::jstr_chances)] = x.m_chances;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, test_c<t_integer_type, t_probability_type>& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <test_c> failed: " + j.dump());
        } // from_json(...)
    }; // struct test_c
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!noexcept_json::optional(j, result_type::jstr_age, x.m_age)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_names, x.m_names)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_tag, x.m_tag)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_chances, x.m_chances)) return false;

            if (x.error_message().has_value()) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <ropufu::integer t_integer_type, std::floating_point t_probability_type>
    struct hash<ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_c<t_integer_type, t_probability_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / 4;
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::age_type> age_hasher = {};
            std::hash<typename argument_type::names_type> names_hasher = {};
            std::hash<typename argument_type::tag_type> tag_hasher = {};
            std::hash<typename argument_type::chances_type> chances_hasher = {};

            result ^= (age_hasher(x.m_age) << ((shift * 0) % total_width));
            result ^= (names_hasher(x.m_names) << ((shift * 1) % total_width));
            result ^= (tag_hasher(x.m_tag) << ((shift * 2) % total_width));
            result ^= (chances_hasher(x.m_chances) << ((shift * 3) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_C_83C6F7C946734848BB292B2949FAD1E9
