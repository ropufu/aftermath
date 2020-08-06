
#ifndef ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED

namespace ropufu::aftermath
{
    /** A place for some mathematical constants. */
    template <typename t_numeric_type>
    struct math_constants
    {
        /** The number 1/2. */
        static constexpr t_numeric_type one_half = t_numeric_type(0.5L);
        /** The number e. */
        [[deprecated("Use std::numbers::e_v<t_numeric_type> instead.")]]
        static constexpr t_numeric_type e = t_numeric_type(2.718281828459045235360287471L);
        /** √e. */
        static constexpr t_numeric_type root_e = t_numeric_type(1.648721270700128146848650787814L);
        /** √(e - 1). */
        static constexpr t_numeric_type root_e_minus_one = t_numeric_type(1.3108324944320861759067703611346874340006L);
        /** The number π. */
        [[deprecated("Use std::numbers::pi_v<t_numeric_type> instead.")]]
        static constexpr t_numeric_type pi = t_numeric_type(3.141592653589793238462643383L);
        /** √π. */
        static constexpr t_numeric_type root_pi = t_numeric_type(1.7724538509055160272981674833411L);
        /** √π / 2. */
        static constexpr t_numeric_type root_pi_div_two = t_numeric_type(0.88622692545275801364908374167057L);
        /** √2. */
        [[deprecated("Use std::numbers::sqrt2_v<t_numeric_type> instead.")]]
        static constexpr t_numeric_type root_two = t_numeric_type(1.4142135623730950488016887242097L);
        /** 1 / √2. */
        static constexpr t_numeric_type one_div_root_two = t_numeric_type(0.70710678118654752440084436210485L);
        /** 1 / √(2 π). */
        static constexpr t_numeric_type one_div_root_two_pi = t_numeric_type(0.39894228040143267793994605993438L);
        /** 2 / √π. */
        static constexpr t_numeric_type two_div_root_pi = t_numeric_type(1.1283791670955125738961589031215L);
        /** 2 to the power 32 (also known as 4294967296). */
        static constexpr t_numeric_type two_pow_32 = t_numeric_type(4294967296L);
        /** √12. */
        static constexpr t_numeric_type root_twelwe = t_numeric_type(3.4641016151377545870548926830117L);
        /** 1 / √12. */
        static constexpr t_numeric_type one_over_root_twelwe = t_numeric_type(0.28867513459481288225457439025098L);
        /** 1 / 12. */
        static constexpr t_numeric_type one_over_twelwe = t_numeric_type(0.08333333333333333333333333333333L);
    }; // struct math_constants
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
