
#ifndef ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"

#include "interval_spacing.hpp"

#include <concepts>   // std::totally_ordered
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <initializer_list> // std::initializer_list
#include <iostream>   // std::ostream
#include <ranges>     // std::ranges::range
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>     // std::vector

namespace ropufu::aftermath::algebra
{
    template <std::totally_ordered t_value_type>
    struct interval;

    template <std::totally_ordered t_value_type>
    void to_json(nlohmann::json& j, const interval<t_value_type>& x) noexcept;
    template <std::totally_ordered t_value_type>
    void from_json(const nlohmann::json& j, interval<t_value_type>& x);

    /** @brief Inspired by MATLAB's linspace function. */
    template <std::totally_ordered t_value_type>
    struct interval
    {
        using type = interval<t_value_type>;
        using value_type = t_value_type;

        // ~~ Json names ~~
        static constexpr char jstr_from[] = "from";
        static constexpr char jstr_to[] = "to";

    private:
        value_type m_from = {};
        value_type m_to = {};

    public:
        interval() noexcept { }
        
        interval(const value_type& from, const value_type& to) noexcept
            : m_from(from), m_to(to)
        {
        } // interval(...)

        interval(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            if (j.is_array())
            {
                std::vector<value_type> interval_pair = {};
                aftermath::noexcept_json::as(j, interval_pair, ec);
                if (ec.value() != 0) return;
                if (interval_pair.size() != 2) // Vector representation must have two entries.
                {
                    ec = std::make_error_code(std::errc::bad_message);
                    return;
                } // if (...)
                this->m_from = interval_pair.front();
                this->m_to = interval_pair.back();
            } // if (...)
            else
            {
                aftermath::noexcept_json::required(j, type::jstr_from, this->m_from, ec);
                aftermath::noexcept_json::required(j, type::jstr_to, this->m_to, ec);
            } // else (...)
        } // interval(...)

        const value_type& from() const noexcept { return this->m_from; }
        const value_type& to() const noexcept { return this->m_to; }

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
            return os << self.m_from << "--" << self.m_to;
        } // operator <<(...)
    }; // struct interval

    // ~~ Json name definitions ~~
    template <std::totally_ordered t_value_type> constexpr char interval<t_value_type>::jstr_from[];
    template <std::totally_ordered t_value_type> constexpr char interval<t_value_type>::jstr_to[];
    
    template <std::totally_ordered t_value_type>
    void to_json(nlohmann::json& j, const interval<t_value_type>& x) noexcept
    {
        using type = interval<t_value_type>;

        j = nlohmann::json{
            {type::jstr_from, x.from()},
            {type::jstr_to, x.to()}
        };
    } // to_json(...)

    template <std::totally_ordered t_value_type>
    void from_json(const nlohmann::json& j, interval<t_value_type>& x)
    {
        using type = interval<t_value_type>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec.value() != 0) throw std::runtime_error("Parsing <interval> failed: " + j.dump());
    } // from_json(...)

    template <std::totally_ordered t_value_type, ropufu::spacing t_spacing_type>
    void explode(const interval<t_value_type>& interval, std::vector<t_value_type>& container, std::size_t count, const t_spacing_type& spacing) noexcept
    {
        using intermediate_type = typename t_spacing_type::intermediate_type;

        switch (count)
        {
            case 0: container = {}; return;
            case 1: container = { interval.from() }; return;
            case 2: container = { interval.from(), interval.to() }; return;
        } // switch (...)

        container.clear();
        container.reserve(count);

        intermediate_type f_from = spacing.forward_transform(interval.from());
        intermediate_type f_to = spacing.forward_transform(interval.to());
        intermediate_type f_diameter = f_to - f_from;
        
        container.push_back(interval.from()); // First value is always the left end-point.
        for (std::size_t i = 1; i < count - 1; ++i)
        {
            intermediate_type f_step = (i * f_diameter) / (count - 1);
            intermediate_type x = spacing.backward_transform(f_from + f_step);

            container.push_back(x);
        } // for (...)
        container.push_back(interval.to()); // Last value is always the right end-point.
        container.shrink_to_fit();
    } // explode(...)
} // namespace ropufu::aftermath::algebra

namespace std
{
    template <std::totally_ordered t_value_type>
    struct hash<ropufu::aftermath::algebra::interval<t_value_type>>
    {
        using argument_type = ropufu::aftermath::algebra::interval<t_value_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_value_type> value_hash {};
            return
                (value_hash(x.from()) << 4) ^ 
                (value_hash(x.to()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_HPP_INCLUDED
