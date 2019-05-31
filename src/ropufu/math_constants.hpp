
#ifndef ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED

namespace ropufu::aftermath
{
    /** A place for some mathematical constants. */
    template <typename t_numeric_type>
    struct math_constants
    {
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

    /** Checks if \tparam t_number is a power of 2. */
    template <typename t_integer_type>
    inline constexpr bool is_power_of_two(t_integer_type number)
    {
        if (number <= 0) return false;
        return ((number & (number - 1)) == 0);
    } // is_power_of_two(...)

    // /** Checks if \tparam t_number is a power of 2. */
    // template <std::size_t t_number>
    // struct is_power_of_two
    // {
    //     static constexpr bool value = ((t_number & (t_number - 1)) == 0);
    //     static constexpr bool is_not_defined = false;
    // }; // struct is_power_of_two
    
    // /** Trivial type with value <false>. */
    // template <>
    // struct is_power_of_two<0>
    // {
    //     static constexpr bool value = false;
    //     static constexpr bool is_not_defined = false;
    // }; // struct is_power_of_two<...>

    /** @breief Finds the largest integer <x> such that 2 to the power <x> is less than or equal to \tparam t_number.
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
    
    // /** Finds the largest integer <x> such that 2 to the power <x> is less than or equal to \tparam t_number. */
    // template <std::size_t t_number>
    // struct log_base_two
    // {
    //     static constexpr std::size_t value = 1 + log_base_two<(t_number >> 1)>::value;
    //     static constexpr bool is_not_defined = false;
    // }; // struct log_base_two

    // /** Trivial type with value <0>. */
    // template <>
    // struct log_base_two<1>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = false;
    // }; // struct log_base_two<...>

    // /** Trivial type with value <0> and \c is_not_defined flag. */
    // template <>
    // struct log_base_two<0>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = true;
    // }; // struct log_base_two<...>
    

    /** @brief Finds the largest integer <x> such that \tparam t_base to the power <x> is less than or equal to \tparam t_number.
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

    // /** Finds the largest integer <x> such that \tparam t_base to the power <x> is less than or equal to \tparam t_number. */
    // template <std::size_t t_number, std::size_t t_base = 10>
    // struct log_base_n
    // {
    //     static constexpr std::size_t value = 1 + log_base_n<(t_number / t_base), t_base>::value;
    //     static constexpr bool is_not_defined = false;
    // }; // struct log_base_n

    // /** Trivial type with value <0>. */
    // template <std::size_t t_base>
    // struct log_base_n<1, t_base>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = false;
    // }; // struct log_base_n<...>

    // /** Trivial type with value <0> and \c is_not_defined flag. */
    // template <std::size_t t_base>
    // struct log_base_n<0, t_base>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = true;
    // }; // struct log_base_n<...>

    // /** Trivial type with value <0> and \c is_not_defined flag. */
    // template <std::size_t t_number>
    // struct log_base_n<t_number, 0>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = true;
    // }; // struct log_base_n<...>

    // /** Trivial type with value <0> and \c is_not_defined flag. */
    // template <std::size_t t_number>
    // struct log_base_n<t_number, 1>
    // {
    //     static constexpr std::size_t value = 0;
    //     static constexpr bool is_not_defined = true;
    // }; // struct log_base_n<...>

    /** Raises \p t_base to the power \p t_power. */
    template <typename t_integer_type, typename t_power_type>
    inline constexpr t_integer_type npow(t_integer_type base, t_power_type power)
    {
        if (base == 0) return 0;
        if (power < 0) return 0;

        t_integer_type result = 1;
        for (std::size_t i = 0; i < power; ++i) result *= base;
        return result;
    } // npow(...)

    // /** Raises \p t_base to the power \p t_power. */
    // template <std::size_t t_base, std::size_t t_power>
    // struct npow
    // {
    //     static constexpr std::size_t value = t_base * npow<t_base, t_power - 1>::value;
    // }; // struct npow

    // /** Trivial type with value <1>. */
    // template <std::size_t t_base>
    // struct npow<t_base, 0>
    // {
    //     static constexpr std::size_t value = 1;
    // }; // struct npow<...>

    // /** A number of the form 2 to the power \t t_power minus 1. */
    // template <std::size_t t_power>
    // struct mersenne_number
    // {
    //     static constexpr std::size_t value = 2 * mersenne_number<t_power - 1>::value + 1;
    // }; // struct mersenne_number

    // /** Trivial type with value <0>. */
    // template <>
    // struct mersenne_number<0>
    // {
    //     static constexpr std::size_t value = 0;
    // }; // struct mersenne_number<...>

    // /** Calculates the product (\p t_number) · (\p t_number - 1) ··· (\p t_number - \p t_count + 1). */
    // template <std::size_t t_number, std::size_t t_count>
    // struct falling_factorial
    // {
    //     static constexpr std::size_t value = t_number * falling_factorial<t_number - 1, t_count - 1>::value;
    // }; // struct falling_factorial

    // /** Type with value \p t_number. */
    // template <std::size_t t_number>
    // struct falling_factorial<t_number, 1>
    // {
    //     static constexpr std::size_t value = t_number;
    // }; // struct falling_factorial<...>

    // /** Trivial type with value <1>. */
    // template <std::size_t t_number>
    // struct falling_factorial<t_number, 0>
    // {
    //     static constexpr std::size_t value = 1;
    // }; // struct falling_factorial<...>

    // /** Calculates the factorial \p t_number! = (\p t_number) · (\p t_number - 1) ··· (2) · (1). */
    // template <std::size_t t_number>
    // struct factorial : public falling_factorial<t_number, t_number> { };

    // /** Calculates the binomical coefficient (\p t_total choose \p t_choose). */
    // template <std::size_t t_total, std::size_t t_choose>
    // struct nchoosek
    // {
    //     static constexpr std::size_t value = (falling_factorial<t_total, t_choose>::value) / (falling_factorial<t_choose, t_choose>::value);
    // }; // struct nchoosek

    // /** Trivial type with value <1>. */
    // template <std::size_t t_total>
    // struct nchoosek<t_total, 0>
    // {
    //     static constexpr std::size_t value = 1;
    // }; // struct nchoosek<...>

    // /** Trivial type with value <1>. */
    // template <std::size_t t_total>
    // struct nchoosek<t_total, t_total>
    // {
    //     static constexpr std::size_t value = 1;
    // }; // struct nchoosek<...>
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
