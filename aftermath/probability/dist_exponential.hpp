
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>
#include <cstddef>
#include <functional> // For std::hash.
#include <random>
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Exponential distribution. */
            struct dist_exponential;

            template <>
            struct is_continuous<dist_exponential>
            {
                using distribution_type = dist_exponential;
                static constexpr bool value = true;
            };

            /** @brief Exponential distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            struct dist_exponential
            {
                using type = dist_exponential;
                using result_type = double;
                using std_type = std::exponential_distribution<result_type>;

                static const std::string name;
                static const dist_exponential standard;

            private:
                double m_mu;
                double m_lambda;
                double m_cache_variance;

            public:
                /** Default constructor with expected value 1. */
                dist_exponential() noexcept : dist_exponential(1) { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_exponential(const std_type& distribution) noexcept : dist_exponential(distribution.lambda()) { }

                /** @brief Constructs an exponential distribution from the expected value.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p rate_of_events is not positive.
                 */
                explicit dist_exponential(double rate_of_events) noexcept
                    : m_mu(1 / rate_of_events), m_lambda(rate_of_events),
                    m_cache_variance(1 / (rate_of_events * rate_of_events))
                {
                    if (rate_of_events <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "<rate_of_events> must be positive.", __FUNCTION__, __LINE__);
                    }
                }

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std_type(this->m_lambda);
                }
                
                /** Rate of the exponential distribution. */
                double rate() const noexcept { return this->m_lambda; }

                /** Expected value of the distribution. */
                double expected_value() const noexcept { return this->m_mu; }
                /** Variance of the distribution. */
                double variance() const noexcept { return this->m_cache_variance; }
                /** Standard deviation of the distribution. */
                double standard_deviation() const noexcept { return this->m_mu; }

                /** Expected value of the distribution. */
                double mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                double stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */                
                double cdf(double x) const noexcept { return x < 0 ? 0 : (1 - std::exp(-this->m_lambda * x)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return x < 0 ? 0 : (this->m_lambda * std::exp(-this->m_lambda * x)); }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return this->m_mu == other.m_mu;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_exponential::name = "exp";
            
            /** Standard exponential distribution. */
            const dist_exponential dist_exponential::standard = dist_exponential(1);
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_exponential>
    {
        using argument_type = ropufu::aftermath::probability::dist_exponential;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return 
                double_hash(x.expected_value());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
