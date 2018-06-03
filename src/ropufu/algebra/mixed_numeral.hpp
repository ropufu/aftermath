
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED

#include "../math_constants.hpp"

#include <cmath>   // std::round
#include <cstddef> // std::size_t
#include <ostream> // std::ostream

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        /** Count decimal digits requred for mixed numeral notation. */
        template <std::size_t t_denominator>
        inline constexpr std::size_t count_digits = (t_denominator == 0) ? 0 : (1 + aftermath::log_base_n(t_denominator - 1, 10));
        
        /** Special case of mixed numeral when no fractional part is required. */
        template <typename t_float_type>
        struct not_mixed_numeral
        {
            using type = not_mixed_numeral;
            using float_type = t_float_type;

            static constexpr std::size_t denominator = 0;
            static constexpr std::size_t decimal_width = 0;

        private:
            bool m_is_negative = false; // Captures the sign of the number.
            std::size_t m_whole = 0; // The whole part of the number.

        public:
            /** Creates a mixed numeral representation of \p value. */
            explicit not_mixed_numeral(float_type value) noexcept
                : m_is_negative(value < 0), m_whole(static_cast<std::size_t>(value < 0 ? (-value) : value))
            {
            } // not_mixed_numeral(...)

            /** Readable output for mixed numerals. */
            friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
            {
                if (self.m_is_negative) os << '-'; // Preceed with the sign, if necessary.
                os << self.m_whole; // Output the whole part of the number.
                return os;
            } // operator <<(...)
        }; // struct not_mixed_numeral
    } // namespace detail

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
    template <std::size_t t_denominator, typename t_float_type = double>
    struct mixed_numeral
    {
        using type = mixed_numeral<t_denominator, t_float_type>;
        using float_type = t_float_type;

        static constexpr std::size_t denominator = t_denominator;
        static constexpr std::size_t decimal_width = detail::count_digits<t_denominator>;

    private:
        bool m_is_negative = false; // Captures the sign of the number.
        std::size_t m_whole = 0; // The whole part of the number.
        std::size_t m_numerator = 0; // The numerator of the fractional part of the number.

    public:
        /** Creates a mixed numeral representation of \p value. */
        explicit mixed_numeral(float_type value) noexcept
            : m_is_negative(value < 0)
        {
            if (value < 0) value = -value; // The sign has been captured already.

            this->m_whole = static_cast<std::size_t>(value); // Store the integer part of \p value.
            value -= this->m_whole; // Discard the integer part.
            value *= denominator; // Upscale by a factor of \tparam t_denominator.
            this->m_numerator = static_cast<std::size_t>(std::round(value)); // Round the result towards the closest integer.
            if (this->m_numerator == t_denominator) // If it was rounded up and hit the next whole number, adjust.
            {
                this->m_numerator = 0;
                this->m_whole++;
            } // if (...)
        } // mixed_numeral(...)

        /** Readable output for mixed numerals. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            if (self.m_is_negative) os << '-'; // Preceed with the sign, if necessary.
            os << self.m_whole; // Output the whole part of the number.
            os << '_';

            // First, take care of padding.
            std::size_t ten = aftermath::npow(10, type::decimal_width - 1);
            while (ten != 1 && self.m_numerator / ten == 0)
            {
                os << '0';
                ten /= 10;
            }
            // Finally, output the numerator of the fractional part.
            os << self.m_numerator << '/' << denominator;
        
            return os;
        } // operator <<(...)
    }; // struct mixed_numeral
    
    /** Special case of mixed numeral when no fractional part is required. */
    template <typename t_float_type>
    struct mixed_numeral<0, t_float_type> : public detail::not_mixed_numeral<t_float_type>
    {
        using type = mixed_numeral<0, t_float_type>;
        using float_type = t_float_type;
    }; // struct mixed_numeral<...>
    
    /** Special case of mixed numeral when no fractional part is required. */
    template <typename t_float_type>
    struct mixed_numeral<1, t_float_type> : public detail::not_mixed_numeral<t_float_type>
    {
        using type = mixed_numeral<1, t_float_type>;
        using float_type = t_float_type;
    }; // struct mixed_numeral<...>
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MIXED_NUMERAL_HPP_INCLUDED
