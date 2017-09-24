
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED

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
                typedef dist_exponential distribution_type;
                static constexpr bool value = true;
            };

            /** Exponential distribution. */
            struct dist_exponential
            {
                typedef dist_exponential type;
                typedef double result_type;

                static const std::string name;
                static const dist_exponential standard;

            private:
                double m_mu;
                double m_lambda;
                double m_cache_variance;

            public:
                /** Default constructor with expected value 1. */
                dist_exponential() noexcept
                    : m_mu(1.0), m_lambda(1.0),
                    m_cache_variance(1.0)
                {
                }

                /** @brief Constructs an exponential distribution from the expected value.
                 *  @exception std::out_of_range \p expected_value is not positive.
                 */
                explicit dist_exponential(double expected_value)
                    : m_mu(expected_value), m_lambda(1.0 / expected_value),
                    m_cache_variance(expected_value * expected_value)
                {
                    if (expected_value <= 0.0) throw std::out_of_range("<expected_value> must be positive.");
                }

                std::exponential_distribution<double> to_std() const noexcept
                {
                    return std::exponential_distribution<double>(this->m_lambda);
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
                double cdf(double x) const noexcept { return x < 0.0 ? 0.0 : (1.0 - std::exp(-this->m_lambda * x)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return x < 0.0 ? 0.0 : (this->m_lambda * std::exp(-this->m_lambda * x)); }

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
            const dist_exponential dist_exponential::standard = dist_exponential(1.0);
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_exponential>
    {
        typedef ropufu::aftermath::probability::dist_exponential argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return 
                double_hash(x.expected_value());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
