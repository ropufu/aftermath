
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED

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
            /** Binomial distribution. */
            struct dist_binomial;

            template <>
            struct is_discrete<dist_binomial>
            {
                typedef dist_binomial distribution_type;
                static constexpr bool value = true;
            };

            /** Binomial distribution. */
            struct dist_binomial
            {
                typedef dist_binomial type;
                typedef std::size_t result_type;

                static const std::string name;

            private:
                result_type m_number_of_trials;
                double m_probability_of_success;
                double m_cache_probability_of_failure, m_cache_expected_value;

            public:
                /** Default constructor with one trial and probability of success 1/2. */
                dist_binomial() noexcept
                    : m_number_of_trials(1), m_probability_of_success(0.5),
                    m_cache_probability_of_failure(0.5), m_cache_expected_value(0.5)
                {
                }

                /** @brief Constructs a binomial distribution from the number of trials, \p n, and probability of success.
                 *  @exception std::out_of_range \p probability_of_success is not in the interval [0, 1].
                 */
                explicit dist_binomial(result_type n, double probability_of_success)
                    : m_number_of_trials(n), m_probability_of_success(probability_of_success),
                    m_cache_probability_of_failure(1.0 - probability_of_success),
                    m_cache_expected_value(n * probability_of_success)
                {
                    if (probability_of_success < 0.0 || probability_of_success > 1.0) throw std::out_of_range("<probability_of_success> must be in the range from 0 to 1");
                }

                std::binomial_distribution<std::size_t> to_std() const noexcept
                {
                    return std::binomial_distribution<std::size_t>(this->m_number_of_trials, this->m_probability_of_success);
                }

                /** Number of trials in the experiment. */
                result_type number_of_trials() const noexcept { return this->m_number_of_trials; }
                /** Probability of success. */
                double probability_of_success() const noexcept { return this->m_probability_of_success; }
                /** Probability of failure. */
                double probability_of_failure() const noexcept { return this->m_cache_probability_of_failure; }

                /** Expected value of the distribution. */
                double expected_value() const noexcept { return this->m_cache_expected_value; }
                /** Variance of the distribution. */
                double variance() const noexcept { return this->m_cache_expected_value * this->m_cache_probability_of_failure; }
                /** Standard deviation of the distribution. */
                double standard_deviation() const noexcept { return std::sqrt(this->variance()); }

                /** Expected value of the distribution. */
                double mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                double stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */
                double cdf(result_type k) const noexcept
                {
                    if (k >= this->m_number_of_trials) return 1.0;
                    
                    double p = 0.0;
                    for (result_type j = 0; j <= k; j++) p += this->pdf(j);
                    return p; 
                }

                /** Point mass function (p.m.f.) of the distribution. */
                double pdf(result_type k) const noexcept
                {
                    return dist_binomial::n_choose_k(this->m_number_of_trials, k) * std::pow(this->m_probability_of_success, k) * std::pow(this->m_cache_probability_of_failure, this->m_number_of_trials - k);
                }

                /** Binomial coefficient. */
                static double n_choose_k(result_type n, result_type k) noexcept
                {
                    if (k > n) return 0.0;
                    if (k > n / 2) k = n - k;

                    double result = 1.0;
                    result_type numerator = n - k;
                    for (result_type i = 1; i <= k; i++) result *= static_cast<double>(++numerator) / i;
                    return result;
                }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_number_of_trials == other.m_number_of_trials &&
                        this->m_probability_of_success == other.m_probability_of_success;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_binomial::name = "binomial";
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_binomial>
    {
        typedef ropufu::aftermath::probability::dist_binomial argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<std::size_t> size_hash = {};
            std::hash<double> double_hash = {};

            return
                size_hash(x.number_of_trials()) ^ 
                double_hash(x.probability_of_success());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED
