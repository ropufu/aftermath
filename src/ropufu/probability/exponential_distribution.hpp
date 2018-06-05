
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::exp
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::exponential_distribution
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval

namespace ropufu::aftermath::probability
{
    /** Exponential distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct exponential_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<exponential_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Exponential distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct exponential_distribution
    {
        using type = exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::exponential_distribution<t_value_type>;

        static constexpr char name[] = "exp"; // "exponential"
        static const type standard;

    private:
        expectation_type m_lambda = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_mu = 1;
        expectation_type m_cache_variance = 1;

        bool validate(std::error_code& ec) const noexcept
        {
            if (std::isnan(this->m_lambda) || std::isinf(this->m_lambda) || this->m_lambda <= 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Rate must be positive.", false);
            return true;
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_mu = 1 / this->m_lambda;
            this->m_cache_variance = this->m_cache_mu * this->m_cache_mu;
        } // coerce(...)

    public:
        /** Default constructor with expected value 1. */
        exponential_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ exponential_distribution(const std_type& distribution) noexcept
            : m_lambda(static_cast<expectation_type>(distribution.lambda()))
        {
            this->cahce();
        } // exponential_distribution(...)

        /** @brief Constructs an exponential distribution from the expected value.
         *  @param ec Set to std::errc::invalid_argument if \p rate_of_events is not positive.
         */
        explicit exponential_distribution(expectation_type rate_of_events, std::error_code& ec) noexcept
            : m_lambda(rate_of_events)
        {
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_lambda = 1;
            } // if (...)
        } // exponential_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_lambda);
        } // to_std(...)
        
        /** Rate of the exponential distribution. */
        expectation_type rate() const noexcept { return this->m_lambda; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_cache_mu; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_cache_mu; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */                
        probability_type cdf(value_type x) const noexcept { return x < 0 ? 0 : static_cast<probability_type>(1 - std::exp(-this->m_lambda * x)); }
        
        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept { return x < 0 ? 0 : static_cast<expectation_type>(this->m_lambda * std::exp(-this->m_lambda * x)); }

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return this->m_lambda == other.m_lambda;
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

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    const exponential_distribution<t_value_type, t_probability_type, t_expectation_type> exponential_distribution<t_value_type, t_probability_type, t_expectation_type>::standard = {};
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
            std::hash<typename argument_type::expectation_type> expectation_hash = {};

            return
                expectation_hash(x.rate());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
