
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED

#include "../template_math.hpp"

#include <cmath>    // For std::round.
#include <cstdint>
#include <ostream>
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        namespace algebra
        {
            namespace detail
            {
                /** Count decimal digits requred for mixed numeral notation. */
                template <std::size_t t_denominator>
                struct count_digits
                {
                    static const std::size_t value = 1 + log_base_n<(t_denominator - 1), 10>::value;
                };

                /** Trivial type with value <0>. */
                template <>
                struct count_digits<0>
                {
                    static const std::size_t value = 0;
                };
                
                /** Special case of mixed numeral when no fractional part is required. */
                struct not_mixed_numeral
                {
                    typedef not_mixed_numeral type;
                    static constexpr std::size_t denominator = 0;
                    static constexpr std::size_t decimal_width = 0;

                private:
                    bool m_is_negative; // Captures the sign of the number.
                    std::size_t m_whole = 0; // The whole part of the number.

                public:
                    /** Creates a mixed numeral representation of a \c double. */
                    explicit not_mixed_numeral(double value) noexcept
                        : m_is_negative(value < 0.0), m_whole(static_cast<std::size_t>(value < 0.0 ? (-value) : value))
                    {
                    }

                    /** Readable output for mixed numerals. */
                    friend std::ostream& operator <<(std::ostream& os, const type& that)
                    {
                        if (that.m_is_negative) os << '-'; // Preceed with the sign, if necessary.
                        os << that.m_whole; // Output the whole part of the number.
                        return os;
                    }
                };
            }

            /** @brief A presenter class for floating-point numbers.
             *
             *  Approximates a positive number (x) with (w + a / d),
             *  where (d) is \tparam t_denominator, (w) is an integer, 
             *  and 0 <= (a) < (d).
             *  In an output stream it will be written as "w_bb...b/d",
             *  where (bb...b) is (a) padded on the left with zeros to
             *  guarantee the same output width of fractional part.
             *
             *  @example The number 2.6 in this notation could be
             *           2_2/3 or 2_2/4, depending on the choice of (d).
             */
            template <std::size_t t_denominator>
            struct mixed_numeral
            {
                typedef mixed_numeral<t_denominator> type;
                static constexpr std::size_t denominator = t_denominator;
                static constexpr std::size_t decimal_width = detail::count_digits<t_denominator>::value;

            private:
                bool m_is_negative; // Captures the sign of the number.
                std::size_t m_whole = 0; // The whole part of the number.
                std::size_t m_numerator = 0; // The numerator of the fractional part of the number.

            public:
                /** Creates a mixed numeral representation of a \c double. */
                explicit mixed_numeral(double value) noexcept
                    : m_is_negative(value < 0.0)
                {
                    if (value < 0.0) value = -value; // The sign has been captured already.

                    this->m_whole = static_cast<std::size_t>(value); // Store the integer part of \p value.
                    value -= this->m_whole; // Discard the integer part.
                    value *= denominator; // Upscale by a factor of \tparam t_denominator.
                    this->m_numerator = static_cast<std::size_t>(std::round(value)); // Round the result towards the closest integer.
                    if (this->m_numerator == t_denominator) // If it was rounded up and hit the next whole number, adjust.
                    {
                        this->m_numerator = 0;
                        this->m_whole++;
                    }
                }

                /** Readable output for mixed numerals. */
                friend std::ostream& operator <<(std::ostream& os, const type& that)
                {
                    if (that.m_is_negative) os << '-'; // Preceed with the sign, if necessary.
                    os << that.m_whole; // Output the whole part of the number.
                    os << '_';

                    // First, take care of padding.
                    std::size_t ten = npow<10, type::decimal_width - 1>::value;
                    while (ten != 1 && that.m_numerator / ten == 0)
                    {
                        os << '0';
                        ten /= 10;
                    }
                    // Finally, output the numerator of the fractional part.
                    os << that.m_numerator << '/' << denominator;
                
                    return os;
                }
            };
            
            /** Special case of mixed numeral when no fractional part is required. */
            template <>
            struct mixed_numeral<0> : public detail::not_mixed_numeral
            {
                typedef mixed_numeral<0> type;
            };
            
            /** Special case of mixed numeral when no fractional part is required. */
            template <>
            struct mixed_numeral<1> : public detail::not_mixed_numeral
            {
                typedef mixed_numeral<1> type;
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED
