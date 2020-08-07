
#ifndef ROPUFU_AFTERMATH_ARITHMETIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ARITHMETIC_HPP_INCLUDED

#include "concepts.hpp"

#include <limits>      // std::numeric_limits
#include <type_traits> // std::type_identity_t

namespace ropufu::aftermath
{
    /** Checks if \p number is a power of 2. */
    template <ropufu::integer t_integer_type>
    inline constexpr bool is_power_of_two(t_integer_type number)
    {
        if constexpr (ropufu::signed_integer<t_integer_type>)
        {
            if (number <= 0) return false;
        } // if constexpr (...)
        else
        {
            if (number == 0) return false;
        } // else (...)
        return ((number & (number - 1)) == 0);
    } // is_power_of_two(...)

    /** @breief Finds the largest integer x such that 2 to the power x is less than or equal to \p number.
     *  @remark When \p number is 0 returns 0.
     */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type log_base_two(t_integer_type number)
    {
        t_integer_type result = 0;
        number /= 2;
        while (number != 0)
        {
            number /= 2;
            ++result;
        } // while (...)
        return result;
    } // log_base_two(...)
    
    /** @brief Finds the largest integer x such that \p base to the power x is less than or equal to \p number.
     *  @remark When \p number is 0 returns 0.
     */
    template <ropufu::integer t_integer_type, typename t_base_type>
    inline constexpr t_integer_type log_base_n(t_integer_type number, t_base_type base)
    {        
        t_integer_type result = 0;
        number /= base;
        while (number != 0)
        {
            number /= base;
            ++result;
        } // while (...)
        return result;
    } // log_base_n(...)

    /** Raises \p base to the power \p power. */
    template <ropufu::integer t_integer_type, ropufu::integer t_power_type>
    inline constexpr t_integer_type npow(t_integer_type base, t_power_type power)
    {
        if (base == 0) return 0;
        if (power < 0) return 0;

        t_integer_type result = 1;
        for (t_power_type i = 0; i < power; ++i) result *= base;
        return result;
    } // npow(...)

    /** A number of the form 2 to the power \p power minus 1. */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type mersenne_number(t_integer_type power)
    {
        if (power < 1) return 0;

        t_integer_type result = 1;
        for (t_integer_type i = 1; i < power; ++i)
        {
            result <<= 1;
            result |= 1;
        } // for (...)
        return result;
    } // mersenne_number(...)

    /** If \p number is a Mersenne number returns its power, otherwise returns 0. */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type mersenne_power(t_integer_type number)
    {
        if (number < 1) return 0;

        t_integer_type result = 0;
        while (number != 0)
        {
            if ((number & 1) == 0) return 0;
            number >>= 1;
            ++result;
        } // while (...)
        return result;
    } // mersenne_number(...)

    /** Calculates the product (\p number) · (\p number - 1) ··· (\p number - \p count + 1). */
    template <typename t_numeric_type, ropufu::integer t_integer_type>
    inline constexpr t_numeric_type falling_factorial(t_numeric_type number, t_integer_type count)
    {
        if (count < 1) return 1;
        t_numeric_type result = number;
        for (t_integer_type i = 1; i < count; ++i)
        {
            --number;
            result *= number;
        } // for (...)
        return result;
    } // falling_factorial(...)

    /** Calculates the product (\p number) · (\p number - 1) ··· (2) · (1). */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type factorial(t_integer_type number) { return falling_factorial(number, number); }

    /** Calculates the binomical coefficient (\p of_total choose \p to_choose). */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type nchoosek(t_integer_type of_total, std::type_identity_t<t_integer_type> to_choose)
    {
        return falling_factorial(of_total, to_choose) / falling_factorial(to_choose, to_choose);
    } // nchoosek(...)

    /** @brief Divides \p numerator by \p denominator and rounds toward zero. */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type fraction_toward_zero(t_integer_type numerator, std::type_identity_t<t_integer_type> denominator) noexcept
    {
        return numerator / denominator;
    } // fraction_toward_zero(...)

    /** @brief Divides \p numerator by \p denominator and rounds away from zero.
     *  @param denominator Should be positive.
     *  @warning Checks on \p denominator being positive are not performed.
     *  @warning \p numerator + \p denominator must not exceed std::numeric_limits<t_integer_type>::max().
     */
    template <ropufu::integer t_integer_type>
    inline constexpr t_integer_type fraction_away_from_zero(t_integer_type numerator, std::type_identity_t<t_integer_type> denominator) noexcept
    {
        if constexpr (ropufu::signed_integer<t_integer_type>)
        {
            return (numerator >= 0) ?
                aftermath::template fraction_toward_zero<t_integer_type>(numerator + (denominator - 1), denominator) :
                -aftermath::template fraction_toward_zero<t_integer_type>(-numerator + (denominator - 1), denominator);
        } // if constexpr (...)
        else return aftermath::template fraction_toward_zero<t_integer_type>(numerator + (denominator - 1), denominator);
    } // fraction_away_from_zero(...)
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_ARITHMETIC_HPP_INCLUDED
