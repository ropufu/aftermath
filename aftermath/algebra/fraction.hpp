
#ifndef ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "factorization.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

namespace ropufu
{
    namespace aftermath
    {
        namespace algebra
        {
            /** @brief Rational numbers, as a fraction of two integers.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_integer_type>
            struct fraction
            {
                using integer_type = t_integer_type;
                using type = fraction<t_integer_type>;

            private:
                integer_type m_numerator = 0; // Numerator of the fraction. Could be negative.
                integer_type m_denominator = 1; // Denominator of the fraction. Always positive.

            public:
                /** Constructs a defult \c fraction with value 0. */
                fraction() noexcept
                {
                }
                
                /** @brief Constructs a \c fraction from an integer \p value. 
                 *  @remark Also defines implicit conversion.
                 */
                fraction(const integer_type& value) noexcept
                    : m_numerator(value), m_denominator(1)
                {
                }

                /** @brief Constructs a \c fraction as a ratio \p numerator / \p denominator.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p denominator is zero.
                 */
                fraction(const integer_type& numerator, const integer_type& denominator) noexcept
                    : m_numerator(numerator), m_denominator(denominator)
                {
                    if (denominator == 0)
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Denominator cannot be zero.", __FUNCTION__, __LINE__);
                        this->m_numerator = 0;
                        this->m_denominator = 1;
                        return;
                    }
                    if (denominator < 0)
                    {
                        this->m_numerator = -this->m_numerator;
                        this->m_denominator = -this->m_denominator;
                    }
                }

                /** Simplifies the fraction. */
                void simplify() noexcept
                {
                    integer_type x = factorization::greatest_common_divisor(this->m_numerator, this->m_denominator);
                    this->m_numerator /= x;
                    this->m_denominator /= x;
                }

                /** Flips the sign of the fraction. */
                void negate() noexcept
                {
                    this->m_numerator = -this->m_numerator;
                }

                /** Replaces the fraction with (1 - this fraction). */
                void subtract_from_one() noexcept
                {
                    this->m_numerator = this->m_denominator - this->m_numerator;
                }

                /** @brief Replaces the fraction with 1 / (this fraction). 
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if this is a zero fraction.
                 */
                void invert() noexcept
                {
                    if (this->m_numerator == 0)
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Cannot invert a zero fraction.", __FUNCTION__, __LINE__);
                        return;
                    }

                    integer_type temp = this->m_denominator;
                    this->m_denominator = this->m_numerator;
                    this->m_numerator = temp;
                }

                /** Numerator of the fraction. */
                const integer_type& numerator() const noexcept { return this->m_numerator; }
                /** Updates the numerator of the fraction. */
                void set_numerator(const integer_type& value) noexcept { this->m_numerator = value; }

                /** Denominator of the fraction. */
                const integer_type& denominator() const noexcept { return this->m_denominator; }
                /** @brief Updates the denominator of the fraction.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p value is zero.
                 */
                void set_denominator(const integer_type& value) noexcept
                {
                    if (value == 0) quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "<value> cannot be zero.", __FUNCTION__, __LINE__);
                    else if (value > 0) this->m_denominator = value;
                    else
                    {
                        // Make sure the denominator is always positive.
                        this->m_numerator = -this->m_numerator;
                        this->m_denominator = -value; 
                    }
                }
                
                /** Casts the fraction to double. */
                explicit operator double() const noexcept { return static_cast<double>(this->m_numerator) / this->m_denominator; }
                
                /** Casts the fraction to float. */
                explicit operator float() const noexcept { return static_cast<float>(this->m_numerator) / this->m_denominator; }

                /** Casts the fraction to its underlying type. */
                explicit operator integer_type() const noexcept
                {
                    return this->m_numerator / this->m_denominator;
                }

                /** Flips the sign of the fraction. */
                type& operator -() noexcept
                {
                    this->negate();
                    return *this;
                }

                /** The sum of \p other and this fraction. */
                type operator +(const integer_type& other) const noexcept
                {
                    return type(this->m_numerator + other * this->m_denominator, this->m_denominator);
                }

                /** Adds \p other to this fraction. */
                type& operator +=(const integer_type& other) noexcept
                {
                    this->m_numerator += other * this->m_denominator;
                    return *this;
                }

                /** The sum of \p left and \p right. */
                friend type operator +(const integer_type& left, const type& right) noexcept
                {
                    return type(left * right.m_denominator + right.m_numerator, right.m_denominator);
                }

                /** The sum of \p other and this fraction. */
                type operator +(const type& other) const noexcept
                {
                    return this->m_denominator == other.m_denominator ?
                        type(this->m_numerator + other.m_numerator, this->m_denominator) :
                        type(this->m_numerator * other.m_denominator + other.m_numerator * this->m_denominator, this->m_denominator * other.m_denominator);
                }

                /** Adds \p other to this fraction. */
                type& operator +=(const type& other) noexcept
                {
                    if (this->m_denominator == other.m_denominator) this->m_numerator += other.m_numerator;
                    else
                    {
                        this->m_numerator = this->m_numerator * other.m_denominator + other.m_numerator * this->m_denominator;
                        this->m_denominator *= other.m_denominator;
                    }
                    return *this;
                }

                /** The difference of this fraction and \p other. */
                type operator -(const integer_type& other) const noexcept
                {
                    return type(this->m_numerator - other * this->m_denominator, this->m_denominator);
                }

                /** Subtracts \p other from this fraction. */
                type& operator -=(integer_type other) noexcept
                {
                    this->m_numerator -= other * this->m_denominator;
                    return *this;
                }

                /** The difference of \p left and \p right. */
                friend type operator -(const integer_type& left, const type& right) noexcept
                {
                    return type(left * right.m_denominator - right.m_numerator, right.m_denominator);
                }

                /** The difference of this fraction and \p other. */
                type operator -(const type& other) const noexcept
                {
                    return this->m_denominator == other.m_denominator ?
                        type(this->m_numerator - other.m_numerator, this->m_denominator) :
                        type(this->m_numerator * other.m_denominator - other.m_numerator * this->m_denominator, this->m_denominator * other.m_denominator);
                }

                /** Subtracts \p other from this fraction. */
                type& operator -=(const type& other) noexcept
                {
                    if (this->m_denominator == other.m_denominator) this->m_numerator -= other.m_numerator;
                    else
                    {
                        this->m_numerator = this->m_numerator * other.m_denominator - other.m_numerator * this->m_denominator;
                        this->m_denominator *= other.m_denominator;
                    }
                    return *this;
                }

                /** The product of \p other and this fraction. */
                type operator *(const integer_type& other) const noexcept
                {
                    return type(this->m_numerator * other, this->m_denominator);
                }

                /** Multiplies this fraction by \p other. */
                type& operator *=(const integer_type& other) noexcept
                {
                    this->m_numerator *= other;
                    return *this;
                }

                /** The product of \p left and \p right. */
                friend type operator *(const integer_type& left, const type& right) noexcept
                {
                    return right * left;
                }

                /** The product of \p other and this fraction. */
                type operator *(const type& other) const noexcept
                {
                    return type(this->m_numerator * other.m_numerator, this->m_denominator * other.m_denominator);
                }

                /** Multiplies this fraction by \p other. */
                type& operator *=(const type& other) noexcept
                {
                    this->m_numerator *= other.m_numerator;
                    this->m_denominator *= other.m_denominator;
                    return *this;
                }

                /** @brief The result of division of this fraction by \p other.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p other is zero.
                 */
                type operator /(const integer_type& other) const noexcept
                {
                    return type(this->m_numerator, this->m_denominator * other);
                }

                /** @brief Divides this fraction by \p other.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p other is zero.
                 */
                type& operator /=(const integer_type& other) noexcept
                {
                    if (other == 0) quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Cannot divide by zero.", __FUNCTION__, __LINE__);
                    else if (other > 0) this->m_denominator *= other;
                    else
                    {
                        this->m_numerator = -this->m_numerator;
                        this->m_denominator *= (-other);
                    }
                    return *this;
                }

                /** @brief The result of division of \p left by \p right.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p right is zero.
                 */
                friend type operator /(const integer_type& left, const type& right) noexcept
                {
                    return type(left * right.m_denominator, right.m_numerator);
                }

                /** @brief The result of division of this fraction by \p other.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p other is zero.
                 */
                type operator /(const type& other) const noexcept
                {
                    return type(this->m_numerator * other.m_denominator, this->m_denominator * other.m_numerator);
                }

                /** @brief Divides this fraction by \p other.
           		 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p other is zero.
                 */
                type& operator /=(const type& other) noexcept
                {
                    if (other.m_numerator == 0) quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Cannot divide by zero.", __FUNCTION__, __LINE__);
                    else if (other.m_numerator > 0)
                    {
                        this->m_numerator *= other.m_denominator;
                        this->m_denominator *= other.m_numerator;
                    }
                    else
                    {
                        this->m_numerator *= (-other.m_denominator);
                        this->m_denominator *= (-other.m_numerator);
                    }
                    return *this;
                }

                /** Determines if this fraction is greater than \p other. */
                bool operator >(const integer_type& other) const noexcept
                {
                    return (this->m_numerator > this->m_denominator * other);
                }

                /** Determines if this fraction is greater than or equal to \p other. */
                bool operator >=(const integer_type& other) const noexcept
                {
                    return (this->m_numerator >= this->m_denominator * other);
                }

                /** Determines if this fraction is less than \p other. */
                bool operator <(const integer_type& other) const noexcept
                {
                    return (this->m_numerator < this->m_denominator * other);
                }

                /** Determines if this fraction is less than or equal to \p other. */
                bool operator <=(const integer_type& other) const noexcept
                {
                    return (this->m_numerator <= this->m_denominator * other);
                }

                /** Determines if \p left is greater than \p right. */
                friend bool operator >(const integer_type& left, const type& right) noexcept
                {
                    return left * right.m_denominator > right.m_numerator;
                }

                /** Determines if \p left is greater than or equal to \p right. */
                friend bool operator >=(const integer_type& left, const type& right) noexcept
                {
                    return left * right.m_denominator >= right.m_numerator;
                }

                /** Determines if \p left is less than \p right. */
                friend bool operator <(const integer_type& left, const type& right) noexcept
                {
                    return left * right.m_denominator < right.m_numerator;
                }

                /** Determines if \p left is less than or equal to \p right. */
                friend bool operator <=(const integer_type& left, const type& right) noexcept
                {
                    return left * right.m_denominator <= right.m_numerator;
                }

                /** Determines if this fraction is greater than \p other. */
                bool operator >(const type& other) const noexcept
                {
                    return (this->m_numerator * other.m_denominator > other.m_numerator * this->m_denominator);
                }

                /** Determines if this fraction is greater than or equal to \p other. */
                bool operator >=(const type& other) const noexcept
                {
                    return (this->m_numerator * other.m_denominator >= other.m_numerator * this->m_denominator);
                }

                /** Determines if this fraction is less than \p other. */
                bool operator <(const type& other) const noexcept
                {
                    return (this->m_numerator * other.m_denominator < other.m_numerator * this->m_denominator);
                }

                /** Determines if this fraction is less than or equal to \p other. */
                bool operator <=(const type& other) const noexcept
                {
                    return (this->m_numerator * other.m_denominator <= other.m_numerator * this->m_denominator);
                }

                /** Determines if this fraction equals \p other. */
                bool operator ==(const type& other) const noexcept
                {
                    return (this->m_numerator * other.m_denominator) == (other.m_numerator * this->m_denominator);
                }

                /** Determines if this fraction does not equal \p other. */
                bool operator !=(const type& other) const noexcept
                {
                    return !(this->operator ==(other));
                }

                /** Output \p that to \p os. */
                friend std::ostream& operator <<(std::ostream& os, const type& that) noexcept
                {
                    return os << that.m_numerator << "/" << that.m_denominator;
                }
            };
        }
    }
}

namespace std
{
    template <typename t_integer_type>
    struct hash<ropufu::aftermath::algebra::fraction<t_integer_type>>
    {
        using argument_type = ropufu::aftermath::algebra::fraction<t_integer_type>;
        using result_type = std::size_t;

        result_type operator()(const argument_type& x) const
        {
            std::hash<t_integer_type> integer_hash = {};
            return
                integer_hash(x.numerator()) ^ 
                integer_hash(x.denominator());
        }
    };
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_FRACTION_HPP_INCLUDED
