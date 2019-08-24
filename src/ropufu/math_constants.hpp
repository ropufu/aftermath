
#ifndef ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED

namespace ropufu::aftermath
{
    /** A place for some mathematical constants. */
    template <typename t_numeric_type>
    struct math_constants
    {
        /** The number 1/2. */
        static constexpr t_numeric_type one_half = t_numeric_type(0.5);
        /** The number e. */
        static constexpr t_numeric_type e = t_numeric_type(2.718281828459045235360287471);
        /** √e. */
        static constexpr t_numeric_type root_e = t_numeric_type(1.648721270700128146848650787814);
        /** √(e - 1). */
        static constexpr t_numeric_type root_e_minus_one = t_numeric_type(1.3108324944320861759067703611346874340006);
        /** The number π. */
        static constexpr t_numeric_type pi = t_numeric_type(3.141592653589793238462643383);
        /** √π. */
        static constexpr t_numeric_type root_pi = t_numeric_type(1.7724538509055160272981674833411);
        /** √π / 2. */
        static constexpr t_numeric_type root_pi_div_two = t_numeric_type(0.88622692545275801364908374167057);
        /** √2. */
        static constexpr t_numeric_type root_two = t_numeric_type(1.4142135623730950488016887242097);
        /** 1 / √2. */
        static constexpr t_numeric_type one_div_root_two = t_numeric_type(0.70710678118654752440084436210485);
        /** 1 / √(2 π). */
        static constexpr t_numeric_type one_div_root_two_pi = t_numeric_type(0.39894228040143267793994605993438);
        /** 2 / √π. */
        static constexpr t_numeric_type two_div_root_pi = t_numeric_type(1.1283791670955125738961589031215);
        /** 2 to the power 32 (also known as 4294967296). */
        static constexpr t_numeric_type two_pow_32 = t_numeric_type(4294967296);

        /** √12. */
        static constexpr t_numeric_type root_twelwe = t_numeric_type(3.4641016151377545870548926830117);
        /** 1 / √12. */
        static constexpr t_numeric_type one_over_root_twelwe = t_numeric_type(0.28867513459481288225457439025098);
        /** 1 / 12. */
        static constexpr t_numeric_type one_over_twelwe = t_numeric_type(0.08333333333333333333333333333333);
    }; // struct math_constants

    /** Checks if \p number is a power of 2. */
    template <typename t_integer_type>
    inline constexpr bool is_power_of_two(t_integer_type number)
    {
        if (number <= 0) return false;
        return ((number & (number - 1)) == 0);
    } // is_power_of_two(...)

    /** @breief Finds the largest integer x such that 2 to the power x is less than or equal to \p number.
     *  @remark When \p number is 0 returns 0.
     */
    template <typename t_integer_type>
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
    template <typename t_integer_type, typename t_base_type>
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
    template <typename t_integer_type, typename t_power_type>
    inline constexpr t_integer_type npow(t_integer_type base, t_power_type power)
    {
        if (base == 0) return 0;
        if (power < 0) return 0;

        t_integer_type result = 1;
        for (t_power_type i = 0; i < power; ++i) result *= base;
        return result;
    } // npow(...)

    /** A number of the form 2 to the power \p power minus 1. */
    template <typename t_integer_type>
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
    template <typename t_integer_type>
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
    template <typename t_numeric_type, typename t_integer_type>
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
    template <typename t_integer_type>
    inline constexpr t_integer_type factorial(t_integer_type number) { return falling_factorial(number, number); }

    /** Calculates the binomical coefficient (\p of_total choose \p to_choose). */
    template <typename t_integer_type>
    inline constexpr t_integer_type nchoosek(t_integer_type of_total, t_integer_type to_choose)
    {
        return falling_factorial(of_total, to_choose) / falling_factorial(to_choose, to_choose);
    } // nchoosek(...)
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
