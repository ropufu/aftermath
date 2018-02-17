
#ifndef ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED

namespace ropufu
{
    namespace aftermath
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
            /** √2. */
            static constexpr t_numeric_type root_two = t_numeric_type(1.4142135623730950488016887242097);
            /** 1 / √(2 pi). */
            static constexpr t_numeric_type one_div_root_two_pi = t_numeric_type(0.39894228040143267793994605993438);
            /** 2 to the power 32 (also known as 4294967296). */
            static constexpr t_numeric_type two_pow_32 = t_numeric_type(4294967296);

            /** √12. */
            static constexpr t_numeric_type root_twelwe = t_numeric_type(3.4641016151377545870548926830117);
            /** 1 / √12. */
            static constexpr t_numeric_type one_over_root_twelwe = t_numeric_type(0.28867513459481288225457439025098);
            /** 1 / 12. */
            static constexpr t_numeric_type one_over_twelwe = t_numeric_type(0.08333333333333333333333333333333);
        };
    }
}

#endif // ROPUFU_AFTERMATH_MATH_CONSTANTS_HPP_INCLUDED
