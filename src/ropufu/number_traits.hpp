
#ifndef ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED

#include <cmath>       // std::isnan, std::isinf
#include <limits>      // std::numeric_limits
#include <type_traits> // std::is_floating_point_v

namespace ropufu::aftermath
{
    template <typename t_value_type>
    struct number_traits
    {
        static bool is_nan(const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isnan(value);
            //else if constexpr (std::numeric_limits<t_value_type>::is_integer) return false;
            else return false;
        } // is_nan(...)

        static bool is_infinite(const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isinf(value);
            //else if constexpr (std::numeric_limits<t_value_type>::is_integer) return false;
            else return false;
        } // is_infinite(...)

        static bool is_finite(const t_value_type& value) noexcept
        {
            if constexpr (std::is_floating_point_v<t_value_type>) return std::isfinite(value);
            else if constexpr (std::numeric_limits<t_value_type>::is_integer) return true;
            else return false;
        } // is_infinite(...)
    }; // struct number_traits<...>

    template <typename t_value_type>
    bool is_nan(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_nan(value); }

    template <typename t_value_type>
    bool is_infinite(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_infinite(value); }

    template <typename t_value_type>
    bool is_finite(const t_value_type& value) noexcept { return number_traits<t_value_type>::is_finite(value); }
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_NUMBER_TRAITS_HPP_INCLUDED
