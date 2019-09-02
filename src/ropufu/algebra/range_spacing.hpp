
#ifndef ROPUFU_AFTERMATH_ALGEBRA_RANGE_SPACING_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_RANGE_SPACING_HPP_INCLUDED

#include "../math_constants.hpp"

#include <cmath>      // std::log, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <initializer_list> // std::initializer_list
#include <iostream>   // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc
#include <type_traits> // ...

namespace ropufu::aftermath::algebra
{
    template <typename t_value_type, typename t_intermediate_type = t_value_type>
    struct linear_spacing
    {
        using type = linear_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            if constexpr (std::is_same_v<intermediate_type, value_type>) return value;
            else return static_cast<intermediate_type>(value);
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            if constexpr (std::is_same_v<intermediate_type, value_type>) return transformed_value;
            else return static_cast<value_type>(transformed_value);
        } // backward_transform(...)
    }; // struct linear_spacing

    /** @remark When constructing ranges based on log-spacing, the choice of base should not matter. */
    template <typename t_value_type, typename t_intermediate_type = double>
    struct logarithmic_spacing
    {
        using type = logarithmic_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;

    private:
        intermediate_type m_log_base = math_constants<intermediate_type>::e;
        intermediate_type m_log_factor = 1;

    public:
        logarithmic_spacing() noexcept { }

        explicit logarithmic_spacing(intermediate_type log_base) noexcept
            : m_log_base(log_base), m_log_factor(1 / std::log(log_base))
        { }

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            return this->m_log_factor * std::log(static_cast<intermediate_type>(value));
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            return static_cast<value_type>(std::pow(this->m_log_base, transformed_value));
        } // backward_transform(...)
    }; // struct logarithmic_spacing

    template <typename t_value_type, typename t_intermediate_type = double>
    struct exponential_spacing
    {
        using type = exponential_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;

    private:
        intermediate_type m_log_base = math_constants<intermediate_type>::e;
        intermediate_type m_log_factor = 1;

    public:
        exponential_spacing() noexcept { }

        explicit exponential_spacing(intermediate_type log_base) noexcept
            : m_log_base(log_base), m_log_factor(1 / std::log(log_base))
        { }

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            return std::pow(this->m_log_base, static_cast<intermediate_type>(value));
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            return static_cast<value_type>(this->m_log_factor * std::log(transformed_value));
        } // backward_transform(...)
    }; // struct exponential_spacing
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_RANGE_SPACING_HPP_INCLUDED
