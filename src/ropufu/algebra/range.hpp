
#ifndef ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../json_traits.hpp"
#include "../on_error.hpp"

#include "../enum_parser.hpp"

#include <cmath>     // std::log10, std::pow
#include <cstddef>   // std::size_t
#include <initializer_list> // std::initializer_list
#include <iostream>  // std::ostream
#include <stdexcept> // std::invalid_argument
#include <string>    // std::string, std::to_string
#include <system_error> // std::error_code, std::errc
#include <vector>    // std::vector

namespace ropufu::aftermath::algebra
{
    enum struct spacing : char
    {
        linear = 0,
        logarithmic = 1,
        exponential = 2
    }; // enum struct spacing
} // namespace ropufu::aftermath::algebra

namespace std
{
    std::string to_string(ropufu::aftermath::algebra::spacing value) noexcept
    {
        using type = ropufu::aftermath::algebra::spacing;

        switch (value)
        {
            case type::linear: return "linear";
            case type::logarithmic: return "logarithmic";
            case type::exponential: return "exponential";
            default: return "unknown";
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    template <>
    struct enum_parser<ropufu::aftermath::algebra::spacing>
    {
        using type = enum_parser<ropufu::aftermath::algebra::spacing>;
        using enum_type = ropufu::aftermath::algebra::spacing;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "linear" || from == "lin") { to = enum_type::linear; return true; }
            if (from == "logarithmic" || from == "log") { to = enum_type::logarithmic; return true; }
            if (from == "exponential" || from == "exp") { to = enum_type::exponential; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser
} // namespace ropufu::aftermath::detail

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename t_value_type, typename t_container_type>
        struct range_container
        {
            using type = range_container<t_value_type, t_container_type>;
            using value_type = t_value_type;
            using container_type = t_container_type;

            static container_type make_empty(std::size_t count = 0) noexcept;
            static container_type make_init(std::initializer_list<value_type>) noexcept;
            static void shrink(container_type& container) noexcept;
        }; // struct range_container

        template <typename t_value_type>
        struct range_container<t_value_type, std::vector<t_value_type>>
        {
            using type = range_container<t_value_type, std::vector<t_value_type>>;
            using value_type = t_value_type;
            using container_type = std::vector<t_value_type>;

            static container_type make_empty(std::size_t count = 0) noexcept { return container_type(count); }
            static container_type make_init(std::initializer_list<value_type> data) noexcept { return container_type(data); }
            static void shrink(container_type& container) noexcept { container.shrink_to_fit(); };
        }; // struct range_container<...>
    } // namespace detail

    /** @brief Inspired by MATLAB's linspace function. */
    template <typename t_value_type>
    struct range
    {
        using type = range<t_value_type>;
        using value_type = t_value_type;

        // ~~ Json names ~~
        static constexpr char jstr_from[] = "from";
        static constexpr char jstr_to[] = "to";

    private:
        value_type m_from = {};
        value_type m_to = {};

    public:
        range() noexcept { }
        
        range(const value_type& from, const value_type& to) noexcept
            : m_from(from), m_to(to)
        {
        } // range(...)

        range(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            if (j.is_array())
            {
                std::vector<value_type> range_pair = {};
                aftermath::noexcept_json::as(j, range_pair, ec);
                if (ec) return;
                if (range_pair.size() != 2) { aftermath::detail::on_error(ec, std::errc::bad_message, "Range should be a vector with two entries."); return; }
                this->m_from = range_pair.front();
                this->m_to = range_pair.back();
            } // if (...)
            else
            {
                aftermath::noexcept_json::required(j, type::jstr_from, this->m_from, ec);
                aftermath::noexcept_json::required(j, type::jstr_to, this->m_to, ec);
                from = j[type::jstr_from];
                to = j[type::jstr_to];
            } // if (...)
        } // range(...)

        const value_type& from() const noexcept { return this->m_from; }
        const value_type& to() const noexcept { return this->m_to; }

        template <typename t_container_type>
        bool explode(t_container_type& container, std::size_t count, spacing transform = spacing::linear) const noexcept
        {
            switch (transform)
            {
                case spacing::linear: return this->explode(container, count, [] (const value_type& x) { return x; }, [] (const value_type& x) { return x; }); // break;
                case spacing::logarithmic: return this->explode(container, count, [] (const value_type& x) { return std::log10(x); }, [] (const value_type& x) { return std::pow(10, x); }); // break;
                case spacing::exponential: return this->explode(container, count, [] (const value_type& x) { return std::pow(10, x); }, [] (const value_type& x) { return std::log10(x); }); // break;
            } // switch (...)
            return false; // Spacing not recognized.
        } // explode(...)

        template <typename t_container_type, typename t_forward_transform_type, typename t_backward_transform_type>
        bool explode(t_container_type& container, std::size_t count, const t_forward_transform_type& forward, const t_backward_transform_type& backward) const noexcept
        {
            using helper_type = detail::range_container<value_type, t_container_type>;

            switch (count)
            {
                case 0: container = helper_type::make_empty(); return true;
                case 1: container = helper_type::make_init({ this->m_from }); return true;
                case 2: container = helper_type::make_init({ this->m_from, this->m_to }); return true;
            } // switch (...)

            container = helper_type::make_empty(count);

            value_type f_from = forward(this->m_from);
            value_type f_to = forward(this->m_to);
            value_type f_range = f_to - f_from;

            std::size_t i = 0;
            bool is_first = true;

            for (value_type& x : container)
            {
                bool is_last = (i == count - 1);

                if (is_first) x = this->m_from;
                else if (is_last) x = this->m_to;
                else
                {
                    value_type f_step = (i * f_range) / (count - 1);
                    x = backward(f_from + f_step);
                }
                ++i;
                is_first = false;
            }
            helper_type::shrink(container);
            return true;
        } // explode(...)

        /** Checks if the two objects are equal. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_from == other.m_from &&
                this->m_to == other.m_to;
        } // operator ==(...)

        /** Checks if the two objects are not equal. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        /** @brief Output to a stream. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct range

    // ~~ Json name definitions ~~
    template <typename t_value_type> constexpr char range<t_value_type>::jstr_from[];
    template <typename t_value_type> constexpr char range<t_value_type>::jstr_to[];
    
    template <typename t_value_type>
    void to_json(nlohmann::json& j, const range<t_value_type>& x) noexcept
    {
        using type = range<t_value_type>;

        j = nlohmann::json{
            {type::jstr_from, x.from()},
            {type::jstr_to, x.to()}
        };
    } // to_json(...)

    template <typename t_value_type>
    void from_json(const nlohmann::json& j, range<t_value_type>& x)
    {
        using type = range<t_value_type>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec) throw std::runtime_error("Parsing failed: " + ec.message());
    } // from_json(...)
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED
