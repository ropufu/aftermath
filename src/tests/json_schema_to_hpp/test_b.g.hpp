
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_018D433A68D9439FB9D0C95A2CE4DD93
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_018D433A68D9439FB9D0C95A2CE4DD93

//=======================================================================
// This file was generated automatically.
// Please do not edit it directly, since any changes may be overwritten.
//=======================================================================
// UTC Date: 2/13/2021 7:55:15 PM
//=======================================================================

#include "test_a.g.hpp"
#include <nlohmann/json.hpp>
#include <ropufu/concepts.hpp>
#include <ropufu/noexcept_json.hpp>
#include <ropufu/number_traits.hpp>

#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <set>         // std::set
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

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

        // ~~ Field typedefs ~~
        using names_type = std::set<std::string>;
        using tag_type = json_schema_to_hpp::test_a;

        // ~~ Json keys ~~
        static constexpr std::string_view jstr_names = "names";
        static constexpr std::string_view jstr_tag = "tag";

        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    protected:
        names_type m_names = {"??"};
        tag_type m_tag = json_schema_to_hpp::test_a{1729};

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

        test_b() { this->validate(); } // Will throw if default values do not pass validation.

        explicit test_b(names_type names)
            : m_names(names)
        {
            this->validate();
        } // test_b(...)

        const names_type& names() const noexcept { return this->m_names; }

        void set_names(const names_type& value)
        {
            this->m_names = value;
            this->validate();
        } // set_names(...)

        const tag_type& tag() const noexcept { return this->m_tag; }

        void set_tag(const tag_type& value) noexcept { this->m_tag = value; }

        /** Checks if this object is equivalent to \param other. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_names == other.m_names &&
                this->m_tag == other.m_tag;
        } // operator ==(...)

        /** Checks if this object is not equivalent to \param other. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_names, x.m_names}
            };

            static type default_instance {};

            if (x.m_tag != default_instance.m_tag) j[std::string(type::jstr_tag)] = x.m_tag;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, test_b<t_integer_type>& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <test_b> failed: " + j.dump());
        } // from_json(...)
    }; // struct test_b
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <ropufu::integer t_integer_type>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!noexcept_json::required(j, result_type::jstr_names, x.m_names)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_tag, x.m_tag)) return false;

            if (x.error_message().has_value()) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <ropufu::integer t_integer_type>
    struct hash<ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_b<t_integer_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / 2;
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::names_type> names_hasher = {};
            std::hash<typename argument_type::tag_type> tag_hasher = {};

            result ^= (names_hasher(x.m_names) << ((shift * 0) % total_width));
            result ^= (tag_hasher(x.m_tag) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_B_018D433A68D9439FB9D0C95A2CE4DD93
