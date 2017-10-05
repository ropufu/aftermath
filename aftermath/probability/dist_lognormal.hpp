
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED

#include "../math_constants.hpp"
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
            /** Lognormal distribution. */
            struct dist_lognormal;

            template <>
            struct is_continuous<dist_lognormal>
            {
                using distribution_type = dist_lognormal;
                static constexpr bool value = true;
            };

            /** @brief Lognormal distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            struct dist_lognormal
            {
                using type = dist_lognormal;
                using result_type = double;
                using std_type = std::lognormal_distribution<result_type>;

                static const std::string name;

            private:
                double m_mu, m_sigma;
                double m_cache_expected_value, m_cache_variance, m_cache_standard_deviation, m_cache_sigma_squared;
                double m_cache_sigma_root_two, m_cache_pdf_scale;

            public:
                /** Default constructor with unit sigma. */
                dist_lognormal() noexcept : dist_lognormal(0, 1) { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_lognormal(const std_type& distribution) noexcept : dist_lognormal(distribution.m(), distribution.s()) { }

                /** @brief Constructs a lognormal distribution from mu and sigma, the mean and standard deviation of the corresponding normal distribution.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p sigma is not positive.
                 */
                explicit dist_lognormal(double mu, double sigma = 1) noexcept
                    : m_mu(mu), m_sigma(sigma),
                    m_cache_expected_value(std::exp(mu + sigma * sigma / 2)),
                    m_cache_variance((std::exp(sigma * sigma) - 1.0) * std::exp(2 * mu + sigma * sigma)),
                    m_cache_standard_deviation(std::sqrt(std::exp(sigma * sigma) - 1.0) * std::exp(mu + sigma * sigma / 2)),
                    m_cache_sigma_squared(sigma * sigma),
                    m_cache_sigma_root_two(sigma * math_constants::root_two), // sigma * std::sqrt(2)
                    m_cache_pdf_scale(math_constants::one_div_root_two_pi / sigma) // 1 / (sigma * std::sqrt(2 pi))
                {
                    if (sigma <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "<sigma> must be positive.", __FUNCTION__, __LINE__);
                    }
                }

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std::lognormal_distribution<double>(this->m_mu, this->m_sigma);
                }

                /** Mean of the underlying normal distribution. */
                double mu() const noexcept { return this->m_mu; }
                /** Standard deviation of the underlying normal distribution. */
                double sigma() const noexcept { return this->m_sigma; }

                /** Expected value of the distribution. */
                double expected_value() const noexcept { return this->m_cache_expected_value; }
                /** Variance of the distribution. */
                double variance() const noexcept { return this->m_cache_variance; }
                /** Standard deviation of the distribution. */
                double standard_deviation() const noexcept { return this->m_cache_standard_deviation; }

                /** Expected value of the distribution. */
                double mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                double stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */                
                double cdf(double x) const noexcept { return std::erfc((this->m_mu - std::log(x)) / (this->m_cache_sigma_root_two)) / 2; }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return (this->m_cache_pdf_scale / x) * std::exp(-(std::log(x) - this->m_mu) * (std::log(x) - this->m_mu) / (2 * this->m_cache_variance)); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                double partial_moment(double a, double b) const noexcept
                {
                    return std::exp(t_nth_moment * this->m_mu + t_nth_moment * t_nth_moment * this->m_cache_sigma_squared / 2) * (
                        std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(a)) / (this->m_cache_sigma_root_two)) -
                        std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(b)) / (this->m_cache_sigma_root_two))) / 2;
                }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_mu == other.m_mu &&
                        this->m_sigma == other.m_sigma;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_lognormal::name = "lognorm";
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_lognormal>
    {
        using argument_type = ropufu::aftermath::probability::dist_lognormal;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.mu()) ^
                double_hash(x.sigma());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED
