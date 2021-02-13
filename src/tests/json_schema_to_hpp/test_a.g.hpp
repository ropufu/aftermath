
#ifndef ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_5F0CBDE0A9C44C5881EE97329453BB63
#define ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_5F0CBDE0A9C44C5881EE97329453BB63

//=======================================================================
// This file was generated automatically.
// Please do not edit it directly, since any changes may be overwritten.
//=======================================================================
// UTC Date: 2/13/2021 7:55:15 PM
//=======================================================================

#include <nlohmann/json.hpp>
#include <ropufu/noexcept_json.hpp>
#include <ropufu/number_traits.hpp>

#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace ropufu::tests::json_schema_to_hpp
{
    struct test_a;

    void to_json(nlohmann::json& j, const test_a& x) noexcept;
    void from_json(const nlohmann::json& j, test_a& x);

    struct test_a
    {
        using type = test_a;

        // ~~ Field typedefs ~~
        using age_type = int;
        using label_type = std::string;

        // ~~ Json keys ~~
        static constexpr std::string_view jstr_age = "age";
        static constexpr std::string_view jstr_label = "label";

        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    protected:
        age_type m_age = 0;
        label_type m_label = "";

    public:
        constexpr void validate() const noexcept { }

        test_a() noexcept { }

        explicit test_a(age_type age) noexcept
            : m_age(age)
        { }

        age_type age() const noexcept { return this->m_age; }

        void set_age(age_type value) noexcept { this->m_age = value; }

        const label_type& label() const noexcept { return this->m_label; }

        void set_label(const label_type& value) noexcept { this->m_label = value; }

        /** Checks if this object is equivalent to \param other. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_age == other.m_age &&
                this->m_label == other.m_label;
        } // operator ==(...)

        /** Checks if this object is not equivalent to \param other. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_age, x.m_age}
            };

            static type default_instance {};

            if (!(x.m_label.empty())) j[std::string(type::jstr_label)] = x.m_label;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, test_a& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <test_a> failed: " + j.dump());
        } // from_json(...)
    }; // struct test_a
} // namespace ropufu::tests::json_schema_to_hpp

namespace ropufu
{
    template <>
    struct noexcept_json_serializer<ropufu::tests::json_schema_to_hpp::test_a>
    {
        using result_type = ropufu::tests::json_schema_to_hpp::test_a;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!noexcept_json::required(j, result_type::jstr_age, x.m_age)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_label, x.m_label)) return false;

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <>
    struct hash<ropufu::tests::json_schema_to_hpp::test_a>
    {
        using argument_type = ropufu::tests::json_schema_to_hpp::test_a;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / 2;
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::age_type> age_hasher = {};
            std::hash<typename argument_type::label_type> label_hasher = {};

            result ^= (age_hasher(x.m_age) << ((shift * 0) % total_width));
            result ^= (label_hasher(x.m_label) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_TESTS_JSON_SCHEMA_TO_HPP_TEST_A_5F0CBDE0A9C44C5881EE97329453BB63
