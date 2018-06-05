
#ifndef ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_REAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_REAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::uniform_real_distribution
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval

namespace ropufu::aftermath::probability
{
    /** Uniform (continuous) distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct uniform_real_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Uniform (continuous) distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct uniform_real_distribution
    {
        using type = uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::uniform_real_distribution<t_value_type>;

        static constexpr char name[] = "uniform";

    private:
        value_type m_a = 0;
        value_type m_b = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_expected_value = static_cast<expectation_type>(0.5);
        expectation_type m_cache_variance = math_constants<expectation_type>::one_over_twelwe;
        expectation_type m_cache_standard_deviation = math_constants<expectation_type>::root_twelwe;
        expectation_type m_cache_length = 1;
        expectation_type m_cache_density = 1;

        bool validate(std::error_code& ec) const noexcept
        {
            bool is_a_not_finite = std::isnan(this->m_a) || std::isinf(this->m_a);
            bool is_b_not_finite = std::isnan(this->m_b) || std::isinf(this->m_b);
            
            if (is_a_not_finite && is_b_not_finite) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Endpoints must be finite.", false);
            else if (is_a_not_finite) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Left endpoint must be finite.", false);
            else if (is_b_not_finite) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Right endpoint must be finite.", false);

            return true;
        } // validate(...)

        void cahce() noexcept
        {
            expectation_type length = static_cast<expectation_type>(this->m_b - this->m_a);
            this->m_cache_expected_value = (this->m_a + this->m_b) / 2;
            this->m_cache_variance = length * length / 12;
            this->m_cache_standard_deviation = length / math_constants<expectation_type>::root_twelwe;
            this->m_cache_length = length;
            this->m_cache_density = 1 / length;
        } // coerce(...)

    public:
        /** Default constructor over the unit interval [0, 1]. */
        uniform_real_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ uniform_real_distribution(const std_type& distribution) noexcept
            : m_a(distribution.a()), m_b(distribution.b())
        {
            this->cahce();
        } // uniform_real_distribution(...)

        /** @brief Constructs a uniform distribution over [\p a, \p b].
         *  @param ec Set to std::errc::invalid_argument if \p b does not exceed \p a.
         */
        explicit uniform_real_distribution(value_type a, value_type b, std::error_code& ec) noexcept
            : m_a(a), m_b(b)
        {
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_a = 0;
                this->m_b = 1;
            } // if (...)
        } // uniform_real_distribution

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_a, this->m_b);
        } // to_std(...)

        /** Left endpoint of the interval. */
        value_type min() const noexcept { return this->m_a; }
        /** Right endpoint of the interval. */
        value_type max() const noexcept { return this->m_b; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_cache_expected_value; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_cache_standard_deviation; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type x) const noexcept { return x < this->m_a ? 0 : static_cast<probability_type>(x >= this->m_b ? 1 : ((x - this->m_a) / this->m_cache_length)); }
        
        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept { return x < this->m_a ? 0 : (x > this->m_b ? 0 : this->m_cache_density); }

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_a == other.m_a &&
                this->m_b == other.m_b;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)
    }; // struct uniform_real_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::uniform_real_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::value_type> value_hash = {};

            return
                value_hash(x.min()) ^
                value_hash(x.max());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_REAL_DISTRIBUTION_HPP_INCLUDED
