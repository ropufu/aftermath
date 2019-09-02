
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../number_traits.hpp"
#include "distribution_traits.hpp"
#include "standard_exponential_distribution.hpp"

#include <cmath>       // std::sqrt, std::pow, std::erfc
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::exponential_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval

namespace ropufu::aftermath::probability
{
    /** @brief Exponential distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct exponential_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<exponential_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct has_right_tail<exponential_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct has_right_tail

    /** @brief Exponential distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct exponential_distribution
    {
        using type = exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using fundamental_type = standard_exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::exponential_distribution<t_value_type>;

        static constexpr char name[] = "exp"; // "exponential"

    private:
        fundamental_type m_fundamental_distribution = {};
        value_type m_lambda = 1; // Rate.
        // ~~ Cached values ~~
        expectation_type m_cache_mean = 1;
        expectation_type m_cache_variance = 1;
        expectation_type m_cache_pdf_scale = 1;

        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_floating_point_v<value_type>, "Value type has to be a floating point type.");
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

        void validate() const
        {
            if (!aftermath::is_finite(this->m_lambda) || this->m_lambda <= 0) throw std::logic_error("Lambda must be positive.");
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_mean = 1 / static_cast<expectation_type>(this->m_lambda);
            this->m_cache_variance = this->m_cache_mean * this->m_cache_mean;
            this->m_cache_pdf_scale = static_cast<expectation_type>(this->m_lambda);
        } // coerce(...)

    public:
        /** Default constructor with with unit mean / rate. */
        exponential_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ exponential_distribution(const std_type& distribution)
            : exponential_distribution(static_cast<value_type>(distribution.lambda()))
        {
        } // exponential_distribution(...)

        /** @brief Constructs a exponential distribution from the mean and standard deviation.
         *  @exception std::logic_error \p lambda is not positive.
         */
        exponential_distribution(value_type lambda)
            : m_lambda(lambda)
        {
            type::traits_check();
            
            this->validate();
            this->cahce();
        } // exponential_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using lambda_type = decltype(std::declval<std_type>().lambda());
            return std_type(static_cast<lambda_type>(this->m_lambda));
        } // to_std(...)

        /** Rate of the distribution. */
        value_type lambda() const noexcept { return this->m_lambda; }
        /** Rate of the distribution. */
        value_type rate() const noexcept { return this->m_lambda; }
        /** Mean of the distribution. */
        value_type mu() const noexcept { return 1 / this->m_lambda; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_cache_mean; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_cache_mean; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */   
        probability_type cdf(value_type x) const noexcept
        {
            return this->m_fundamental_distribution.cdf(this->m_lambda * x);
        } // cdf(...)

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return this->m_cache_pdf_scale * this->m_fundamental_distribution.pdf(this->m_lambda * x);
        } // pdf(...)
        
        /** @brief For a given 0 <= p <= 1, find t such that cdf(t) = p.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         */
        expectation_type quantile(probability_type p) const
        {
            return this->m_cache_mean * this->m_fundamental_distribution.quantile(p);
        } // quantile(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_lambda == other.m_lambda;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)
    }; // struct exponential_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char exponential_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::exponential_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::value_type> value_hash = {};
            return
                (value_hash(x.lambda()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
