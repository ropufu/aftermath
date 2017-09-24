
#ifndef ROPUFU_AFTERMATH_TEMPLATE_MATH_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TEMPLATE_MATH_HPP_INCLUDED

#include <cstddef>
#include <cstdint>

namespace ropufu
{
    /** @todo Rewrite to template on unsigned types other than \c std::size_t. */
    namespace aftermath
    {
        /** Checks if \tparam t_number is a power of 2. */
        template <std::size_t t_number>
        struct is_power_of_two
        {
            static constexpr bool value = ((t_number & (t_number - 1)) == 0);
            static constexpr bool is_not_defined = false;
        };
        
        /** Trivial type with value <false>. */
        template <>
        struct is_power_of_two<0>
        {
            static constexpr bool value = false;
            static constexpr bool is_not_defined = false;
        };

        /** Finds the largest integer <x> such that 2 to the power <x> is less than or equal to \tparam t_number. */
        template <std::size_t t_number>
        struct log_base_two
        {
            static constexpr std::size_t value = 1 + log_base_two<(t_number >> 1)>::value;
            static constexpr bool is_not_defined = false;
        };

        /** Trivial type with value <0>. */
        template <>
        struct log_base_two<1>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = false;
        };

        /** Trivial type with value <0> and \c is_not_defined flag. */
        template <>
        struct log_base_two<0>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = true;
        };

        /** Finds the largest integer <x> such that \tparam t_base to the power <x> is less than or equal to \tparam t_number. */
        template <std::size_t t_number, std::size_t t_base = 10>
        struct log_base_n
        {
            static constexpr std::size_t value = 1 + log_base_n<(t_number / t_base), t_base>::value;
            static constexpr bool is_not_defined = false;
        };

        /** Trivial type with value <0>. */
        template <std::size_t t_base>
        struct log_base_n<1, t_base>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = false;
        };

        /** Trivial type with value <0> and \c is_not_defined flag. */
        template <std::size_t t_base>
        struct log_base_n<0, t_base>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = true;
        };

        /** Trivial type with value <0> and \c is_not_defined flag. */
        template <std::size_t t_number>
        struct log_base_n<t_number, 0>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = true;
        };

        /** Trivial type with value <0> and \c is_not_defined flag. */
        template <std::size_t t_number>
        struct log_base_n<t_number, 1>
        {
            static constexpr std::size_t value = 0;
            static constexpr bool is_not_defined = true;
        };

        /** Raises \p t_base to the power \p t_power. */
        template <std::size_t t_base, std::size_t t_power>
        struct npow
        {
            static constexpr std::size_t value = t_base * npow<t_base, t_power - 1>::value;
        };

        /** Trivial type with value <1>. */
        template <std::size_t t_base>
        struct npow<t_base, 0>
        {
            static constexpr std::size_t value = 1;
        };

        /** A number of the form 2 to the power \t t_power minus 1. */
        template <std::size_t t_power>
        struct mersenne_number
        {
            static constexpr std::size_t value = 2 * mersenne_number<t_power - 1>::value + 1;
        };

        /** Trivial type with value <0>. */
        template <>
        struct mersenne_number<0>
        {
            static constexpr std::size_t value = 0;
        };

        /** Calculates the product (\p t_number) · (\p t_number - 1) ··· (\p t_number - \p t_count + 1). */
        template <std::size_t t_number, std::size_t t_count>
        struct falling_factorial
        {
            static constexpr std::size_t value = t_number * falling_factorial<t_number - 1, t_count - 1>::value;
        };

        /** Type with value \p t_number. */
        template <std::size_t t_number>
        struct falling_factorial<t_number, 1>
        {
            static constexpr std::size_t value = t_number;
        };

        /** Trivial type with value <1>. */
        template <std::size_t t_number>
        struct falling_factorial<t_number, 0>
        {
            static constexpr std::size_t value = 1;
        };

        /** Calculates the factorial \p t_number! = (\p t_number) · (\p t_number - 1) ··· (2) · (1). */
        template <std::size_t t_number, std::size_t t_count>
        struct factorial : public falling_factorial<t_number, t_number>
        {
        };

        /** Calculates the binomical coefficient (\p t_total choose \p t_choose). */
        template <std::size_t t_total, std::size_t t_choose>
        struct nchoosek
        {
            static constexpr std::size_t value = (falling_factorial<t_total, t_choose>::value) / (falling_factorial<t_choose, t_choose>::value);
        };

        /** Trivial type with value <1>. */
        template <std::size_t t_total>
        struct nchoosek<t_total, 0>
        {
            static constexpr std::size_t value = 1;
        };

        /** Trivial type with value <1>. */
        template <std::size_t t_total>
        struct nchoosek<t_total, t_total>
        {
            static constexpr std::size_t value = 1;
        };
    }
}

#endif // ROPUFU_AFTERMATH_TEMPLATE_MATH_HPP_INCLUDED
