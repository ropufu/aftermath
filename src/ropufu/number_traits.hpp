
#ifndef ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED

#include "concepts.hpp"

#include <cmath>       // std::isnan, std::isinf, std::isfinite
#include <limits>      // std::numeric_limits
#include <ranges>      // std::ranges:range
#include <type_traits> // std::is_floating_point_v

namespace ropufu::aftermath
{
    template <typename t_value_type>
    struct number_traits
    {
        static bool is_nan([[maybe_unused]] const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isnan(value);
            else return false;
        } // is_nan(...)

        static bool is_infinite([[maybe_unused]] const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isinf(value);
            else return false;
        } // is_infinite(...)

        static bool is_finite([[maybe_unused]] const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isfinite(value);
            else if constexpr (std::numeric_limits<t_value_type>::is_integer) return true;
            else return false;
        } // is_infinite(...)
    }; // struct number_traits<...>

    template <typename t_value_type>
    inline bool is_nan(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_nan(value); }

    template <typename t_value_type>
    inline bool is_infinite(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_infinite(value); }

    template <typename t_value_type>
    inline bool is_finite(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_finite(value); }

    template <typename t_value_type>
    inline bool is_probability(const t_value_type& value) noexcept { return (aftermath::is_finite(value) && value >= 0 && value <= 1); }

    template <typename t_numeric_type>
        requires ropufu::zero_comparable<t_numeric_type> && ropufu::zero_assignable<t_numeric_type>
    inline void make_non_negative([[maybe_unused]] t_numeric_type& value) noexcept
    {
        if constexpr (!std::numeric_limits<t_numeric_type>::is_signed) return; // Unsigned numbers are implicitly assumed to be positive.
        if (value < 0) value = 0;
    } // make_non_negative(...)

    template <std::ranges::range t_range_type>
    inline void make_non_negative(t_range_type& value) noexcept
    {
        for (auto& x : value) aftermath::make_non_negative(x);
    } // make_non_negative(...)
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED
