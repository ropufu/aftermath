
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED

#include "../math_constants.hpp"
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
            /** Normal (Gaussian) distribution. */
            struct dist_normal;
            using dist_gaussian = dist_normal;

            template <>
            struct is_continuous<dist_normal>
            {
                typedef dist_normal distribution_type;
                static constexpr bool value = true;
            };

            /** Normal (Gaussian) distribution. */
            struct dist_normal
            {
                typedef dist_normal type;
                typedef double result_type;

                static const std::string name;

                static const dist_normal standard;

            private:
                double m_mu, m_sigma;
                double m_cache_variance;
                double m_cache_sigma_root_two, m_cache_pdf_scale;

            public:
                /** Default constructor with zero mean and unit variance. */
                dist_normal() noexcept
                    : m_mu(0.0), m_sigma(1.0),
                    m_cache_variance(1.0),
                    m_cache_sigma_root_two(math_constants::root_two), // sqrt(2)
                    m_cache_pdf_scale(math_constants::one_div_root_two_pi) // 1 / sqrt(2 pi)
                {
                }

                /** @brief Constructs a normal distribution from the mean and standard deviation.
                 *  @exception std::out_of_range \p sigma is not positive.
                 */
                explicit dist_normal(double mu, double sigma = 1.0)
                    : m_mu(mu), m_sigma(sigma),
                    m_cache_variance(sigma * sigma),
                    m_cache_sigma_root_two(sigma * math_constants::root_two), // sigma * sqrt(2)
                    m_cache_pdf_scale(math_constants::one_div_root_two_pi / sigma) // 1 / (sigma * sqrt(2 pi))
                {
                    if (sigma <= 0.0) throw std::out_of_range("<sigma> must be positive.");
                }

                std::normal_distribution<double> to_std() const noexcept
                {
                    return std::normal_distribution<double>(this->m_mu, this->m_sigma);
                }

                /** Mean of the distribution. */
                double mu() const noexcept { return this->m_mu; }
                /** Standard deviation of the distribution. */
                double sigma() const noexcept { return this->m_sigma; }

                /** Expected value of the distribution. */
                double expected_value() const noexcept { return this->m_mu; }
                /** Variance of the distribution. */
                double variance() const noexcept { return this->m_cache_variance; }
                /** Standard deviation of the distribution. */
                double standard_deviation() const noexcept { return this->m_sigma; }

                /** Expected value of the distribution. */
                double mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                double stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */   
                double cdf(double x) const noexcept { return std::erfc((this->m_mu - x) / (this->m_cache_sigma_root_two)) / 2; }

                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return this->m_cache_pdf_scale * exp(-(x - this->m_mu) * (x - this->m_mu) / (2 * this->m_cache_variance)); }

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
            const std::string dist_normal::name = "norm";
            
            /** Standard exponential distribution. */
            const dist_normal dist_normal::standard = dist_normal(0.0);
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_normal>
    {
        typedef ropufu::aftermath::probability::dist_normal argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.mu()) ^
                double_hash(x.sigma());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED
