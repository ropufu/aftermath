
#ifndef ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED

#include "../on_error.hpp"

#include <cstddef>      // std::size_t
#include <functional>   // std::hash
#include <limits>       // std::numeric_limits::is_integer
#include <numeric>      // std::gcd
#include <ostream>      // std::ostream
#include <stdexcept>    // std::logic_error
#include <string>       // std::string
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // ...
#include <utility>      // std::swap

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <bool t_is_enabled, typename t_derived_type>
        struct fraction_negate_module
        {
        protected:
            void regularize() noexcept { }
        }; // struct fraction_negate_module

        template <typename t_derived_type>
        struct fraction_negate_module<true, t_derived_type>
        {
        protected:
            void regularize() noexcept
            {
                t_derived_type* that = static_cast<t_derived_type*>(this);
                if (that->m_denominator < 0)
                {
                    that->m_numerator = -that->m_numerator;
                    that->m_denominator = -that->m_denominator;
                } // if (...)
            } // regularize(...)

        public:
            /** Flips the sign of the fraction. */
            void negate() noexcept
            {
                t_derived_type* that = static_cast<t_derived_type*>(this);
                that->m_numerator = -(that->m_numerator);
            } // negate(...)

            /** Flips the sign of the fraction. */
            t_derived_type& operator -() noexcept
            {
                t_derived_type* that = static_cast<t_derived_type*>(this);
                this->negate();
                return *that;
            } // operator -(...)
        }; // struct fraction_negate_module<...>
    } // namespace detail

    /** @brief Rational numbers, as a fraction of two integers. */
    template <typename t_integer_type>
    struct fraction : public detail::fraction_negate_module<std::is_signed_v<t_integer_type>, fraction<t_integer_type>>
    {
        using type = fraction<t_integer_type>;
        using integer_type = t_integer_type;

        template <bool, typename> friend struct detail::fraction_negate_module;

    private:
        integer_type m_numerator = 0; // Numerator of the fraction. Could be negative.
        integer_type m_denominator = 1; // Denominator of the fraction. Always positive.

        static constexpr void traits_check()
        {
            static_assert(std::numeric_limits<integer_type>::is_integer, "Underlying type has to be an integer type.");
        } // traits_check(...)

    public:
        /** Constructs a defult \c fraction with value 0. */
        fraction() noexcept { type::traits_check(); }
        
        /** @brief Constructs a \c fraction from an integer \p value. 
         *  @remark Also defines implicit conversion.
         */
        /*implicit*/ fraction(const integer_type& value) noexcept
            : m_numerator(value), m_denominator(1)
        {
            type::traits_check();
        } // fraction(...)

        /** @brief Constructs a \c fraction as a ratio \p numerator / \p denominator.
         *  @param ec Set to \c std::errc::invalid_argument if \p denominator is zero.
         */
        fraction(const integer_type& numerator, const integer_type& denominator, std::error_code& ec) noexcept
            : m_numerator(numerator), m_denominator(denominator)
        {
            type::traits_check();
            if (denominator == 0) { aftermath::detail::on_error(ec, std::errc::invalid_argument, "Denominator cannot be zero."); return; }
            this->regularize();
        } // fraction(...)

        /** Simplifies the fraction. */
        void simplify() noexcept
        {
            integer_type x = std::gcd(this->m_numerator, this->m_denominator);
            this->m_numerator /= x;
            this->m_denominator /= x;
        } // simplify(...)

        /** Replaces the fraction with (1 - this fraction). */
        void subtract_from_one() noexcept
        {
            this->m_numerator = this->m_denominator - this->m_numerator;
        } // subtract_from_one(...)

        /** @brief Replaces the fraction with 1 / (this fraction). 
         *  @param ec Set to \c std::function_not_supported if this is a zero fraction.
         */
        void invert(std::error_code& ec) noexcept
        {
            if (this->m_numerator == 0) return aftermath::detail::on_error(ec, std::errc::function_not_supported, "Cannot invert a zero fraction.");
            std::swap(this->m_numerator, this->m_denominator);
            this->regularize();
        } // invert(...)

        /** Numerator of the fraction. */
        const integer_type& numerator() const noexcept { return this->m_numerator; }
        /** Sets the numerator of the fraction. */
        void set_numerator(const integer_type& value) noexcept { this->m_numerator = value; }

        /** Denominator of the fraction. */
        const integer_type& denominator() const noexcept { return this->m_denominator; }
        /** @brief Sets the denominator of the fraction.
         *  @param ec Set to \c std::function_not_supported if \p value is zero.
         */
        void set_denominator(const integer_type& value, std::error_code& ec) noexcept
        {
            if (value == 0) return aftermath::detail::on_error(ec, std::errc::function_not_supported, "Denominator cannot be zero.");
            this->m_denominator = value;
            this->regularize();
        } // set_denominator(...)
        
        /** Casts the fraction to double. */
        explicit operator double() const noexcept { return static_cast<double>(this->m_numerator) / this->m_denominator; }
        
        /** Casts the fraction to float. */
        explicit operator float() const noexcept { return static_cast<float>(this->m_numerator) / this->m_denominator; }

        /** Casts the fraction to its underlying type. */
        explicit operator integer_type() const noexcept
        {
            return this->m_numerator / this->m_denominator;
        } // operator integer_type(...)

        // ~~ Addition ~~
        type& operator +=(const integer_type& other) noexcept { this->m_numerator += other * this->m_denominator; return *this; }
        type& operator +=(const type& other) noexcept
        {
            if (this->m_denominator == other.m_denominator) this->m_numerator += other.m_numerator;
            else
            {
                this->m_numerator = this->m_numerator * other.m_denominator + other.m_numerator * this->m_denominator;
                this->m_denominator *= other.m_denominator;
            }
            return *this;
        } // operator +=(...)

        // ~~ Subtraction ~~
        type& operator -=(const integer_type& other) noexcept { this->m_numerator -= other * this->m_denominator; return *this; }
        friend type operator -(const integer_type& left, type right) noexcept { right.m_numerator = left * right.m_denominator - right.m_numerator; return right; }
        type& operator -=(const type& other) noexcept
        {
            if (this->m_denominator == other.m_denominator) this->m_numerator -= other.m_numerator;
            else
            {
                this->m_numerator = this->m_numerator * other.m_denominator - other.m_numerator * this->m_denominator;
                this->m_denominator *= other.m_denominator;
            }
            return *this;
        } // operator -=(...)

        // ~~ Multiplication ~~
        type& operator *=(const integer_type& other) noexcept { this->m_numerator *= other; return *this; }
        type& operator *=(const type& other) noexcept
        {
            this->m_numerator *= other.m_numerator;
            this->m_denominator *= other.m_denominator;
            return *this;
        } // operator *=(...)
        
        // ~~ Division ~~
        type& operator /=(const integer_type& other)
        {
            if (other == 0) throw std::logic_error("Cannot divide by zero.");
            this->m_denominator *= other;
            this->regularize();
            return *this;
        } // operator /=(...)
        friend type operator /(const integer_type& left, type right) { right.m_denominator *= left; right.invert(); return right; }
        type& operator /=(const type& other)
        {
            if (other.m_numerator == 0) throw std::logic_error("Cannot divide by zero.");
            this->m_numerator *= other.m_denominator;
            this->m_denominator *= other.m_numerator;
            this->regularize();
            return *this;
        } // operator /=(...)

        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators. */
        friend type operator +(type left, const integer_type& right) noexcept { left += right; return left; }
        friend type operator +(const integer_type& left, type right) noexcept { right += left; return right; }
        friend type operator +(type left, const type& right) noexcept { left += right; return left; }
        
        friend type operator -(type left, const integer_type& right) noexcept { left -= right; return left; }
        friend type operator -(type left, const type& right) noexcept { left -= right; return left; }
        
        friend type operator *(type left, const integer_type& right) noexcept { left *= right; return left; }
        friend type operator *(const integer_type& left, type right) noexcept { right *= left; return right; }
        friend type operator *(type left, const type& right) noexcept { left *= right; return left; }

        friend type operator /(type left, const integer_type& right) { left /= right; return left; }
        friend type operator /(type left, const type& right) { left /= right; return left; }


        bool operator >(const integer_type& other) const noexcept { return (this->m_numerator > this->m_denominator * other); }
        bool operator >=(const integer_type& other) const noexcept { return (this->m_numerator >= this->m_denominator * other); }

        bool operator <(const integer_type& other) const noexcept { return (this->m_numerator < this->m_denominator * other); }
        bool operator <=(const integer_type& other) const noexcept { return (this->m_numerator <= this->m_denominator * other); }

        friend bool operator >(const integer_type& left, const type& right) noexcept { return left * right.m_denominator > right.m_numerator; }
        friend bool operator >=(const integer_type& left, const type& right) noexcept { return left * right.m_denominator >= right.m_numerator; }

        friend bool operator <(const integer_type& left, const type& right) noexcept { return left * right.m_denominator < right.m_numerator; }
        friend bool operator <=(const integer_type& left, const type& right) noexcept { return left * right.m_denominator <= right.m_numerator; }

        bool operator >(const type& other) const noexcept { return (this->m_numerator * other.m_denominator > other.m_numerator * this->m_denominator); }
        bool operator >=(const type& other) const noexcept { return (this->m_numerator * other.m_denominator >= other.m_numerator * this->m_denominator); }

        bool operator <(const type& other) const noexcept { return (this->m_numerator * other.m_denominator < other.m_numerator * this->m_denominator); }
        bool operator <=(const type& other) const noexcept { return (this->m_numerator * other.m_denominator <= other.m_numerator * this->m_denominator); }

        bool operator ==(const type& other) const noexcept { return (this->m_numerator * other.m_denominator) == (other.m_numerator * this->m_denominator); }
        bool operator !=(const type& other) const noexcept { return !(this->operator ==(other)); }

        /** Output \p that to \p os. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            return os << self.m_numerator << "/" << self.m_denominator;
        } // operator <<(...)
    }; // struct fraction
} // namespace ropufu::aftermath::algebra

namespace std
{
    template <typename t_integer_type>
    struct hash<ropufu::aftermath::algebra::fraction<t_integer_type>>
    {
        using argument_type = ropufu::aftermath::algebra::fraction<t_integer_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_integer_type> integer_hash {};
            return
                integer_hash(x.numerator()) ^ 
                integer_hash(x.denominator());
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED