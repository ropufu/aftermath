
#ifndef ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif
#include "../noexcept_json.hpp"

#include "interval_spacing.hpp"

#include <concepts>     // std::totally_ordered
#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <initializer_list> // std::initializer_list
#include <iostream>     // std::ostream
#include <ranges>       // std::ranges::range
#include <stdexcept>    // std::logic_error, std::runtime_error
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <vector>       // std::vector

namespace ropufu::aftermath::algebra
{
    template <std::totally_ordered t_value_type>
    struct interval;

#ifndef ROPUFU_NO_JSON
    template <std::totally_ordered t_value_type>
    void to_json(nlohmann::json& j, const interval<t_value_type>& x) noexcept;
    template <std::totally_ordered t_value_type>
    void from_json(const nlohmann::json& j, interval<t_value_type>& x);
#endif

    /** @brief Inspired by MATLAB's linspace function. */
    template <std::totally_ordered t_value_type>
    struct interval
    {
        using type = interval<t_value_type>;
        using value_type = t_value_type;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_from = "from";
        static constexpr std::string_view jstr_to = "to";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif

    private:
        value_type m_from = {};
        value_type m_to = {};

    public:
        interval() noexcept { }
        
        interval(const value_type& from, const value_type& to)
            : m_from(from), m_to(to)
        {
            if (this->m_from > this->m_to) throw std::logic_error("Left endpoint cannot exceed right endpoint.");
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

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_from, x.from()},
                {type::jstr_to, x.to()}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x)) throw std::runtime_error("Parsing <interval> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct interval
} // namespace ropufu::aftermath::algebra

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    template <std::totally_ordered t_value_type>
    struct noexcept_json_serializer<ropufu::aftermath::algebra::interval<t_value_type>>
    {
        using value_type = t_value_type;
        using result_type = ropufu::aftermath::algebra::interval<t_value_type>;

        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (j.is_array()) // [a, b]
            {
                std::vector<value_type> interval_pair = {};
                if (!noexcept_json::try_get(j, interval_pair)) return false;
                if (interval_pair.size() != 2) return false; // Vector representation must have two entries.
                
                x.m_from = interval_pair.front();
                x.m_to = interval_pair.back();
            } // if (...)
            else // {"from": a, "to": b}
            {
                if (!noexcept_json::required(j, result_type::jstr_from, x.m_from)) return false;
                if (!noexcept_json::required(j, result_type::jstr_to, x.m_to)) return false;
            } // else (...)

            if (x.m_from > x.m_to) return false;
            
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

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
