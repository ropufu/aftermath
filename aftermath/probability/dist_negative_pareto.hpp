
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED

#include "../math_constants.hpp"
#include "traits.hpp"

#include <cmath>
#include <cstddef>
#include <functional> // For std::hash.
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Negative Pareto distribution: a special case of scaled (0; x_max) Beta distribution with parameter beta = 1. */
            struct dist_negative_pareto;

            template <>
            struct is_continuous<dist_negative_pareto>
            {
                typedef dist_negative_pareto distribution_type;
                static constexpr bool value = true;
            };

            /** Negative Pareto distribution: special case of scaled (0; x_max) Beta distribution with parameter beta = 1. */
            struct dist_negative_pareto
            {
                typedef dist_negative_pareto type;
                typedef double result_type;

                static const std::string name;

            private:
                double m_alpha, m_x_max;
                double m_cache_expected_value, m_cache_variance, m_cache_standard_deviation;
                double m_cache_axa;

            public:
                /** Default constructor with unit alpha and unit maximum. */
                dist_negative_pareto() noexcept
                    : m_alpha(1.0), m_x_max(1.0),
                    m_cache_expected_value(0.5),
                    m_cache_variance(1.0 / 12),
                    m_cache_standard_deviation(math_constants::one_over_root_twelwe),
                    m_cache_axa(1.0)
                {
                }

                /** @brief Constructs a negative Pareto distribution from \p alpha and \p x_max.
                 *  @exception std::out_of_range \p alpha is not positive.
                 *  @exception std::out_of_range \p x_max is not positive.
                 */
                explicit dist_negative_pareto(double alpha, double x_max)
                    : m_alpha(alpha), m_x_max(x_max),
                    m_cache_expected_value(x_max * alpha / (alpha + 1.0)),
                    m_cache_variance(x_max * x_max / ((alpha + 1.0) * (alpha + 1.0) * (1.0 + 2.0 / alpha))),
                    m_cache_standard_deviation(x_max / ((alpha + 1.0) * std::sqrt(1.0 + 2.0 / alpha))),
                    m_cache_axa(alpha / std::pow(x_max, alpha))
                {
                    if (alpha <= 0.0) throw std::out_of_range("<alpha> must be positive.");
                    if (x_max <= 0.0) throw std::out_of_range("<x_max> must be positive.");
                }

                //... to_std() const noexcept;

                /** Shape parameter of the distribution. */
                double alpha() const noexcept { return this->m_alpha; }
                /** Scale parameter of the distribution. */
                double x_max() const noexcept { return this->m_x_max; }

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
                double cdf(double x) const noexcept { return x <= 0.0 ? 0.0 : (x >= this->m_x_max ? 1.0 : std::pow(x / this->m_x_max, this->m_alpha)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return (x <= 0.0 || x >= this->m_x_max) ? 0.0 : (this->m_cache_axa * std::pow(x, this->m_alpha - 1.0)); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                double partial_moment(double a, double b) const noexcept
                {
                    if (b <= 0.0 || a >= this->m_x_max) return 0.0;
                    if (b > this->m_x_max) b = this->m_x_max;
                    if (a < 0.0) a = 0.0;
                
                    // this->m_cache_axa = this->m_xalpha / std::pow(this->m_x_max, this->m_alpha);
                    return this->m_cache_axa * (std::pow(b, t_nth_moment + this->m_alpha) - std::pow(a, t_nth_moment + this->m_alpha)) / (t_nth_moment + this->m_alpha);
                }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_alpha == other.m_alpha &&
                        this->m_x_max == other.m_x_max;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_negative_pareto::name = "negative_pareto";
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_negative_pareto>
    {
        typedef ropufu::aftermath::probability::dist_negative_pareto argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.alpha()) ^
                double_hash(x.x_max());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED
