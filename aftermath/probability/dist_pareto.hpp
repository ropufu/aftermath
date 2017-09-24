
#ifndef AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
#define AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED

#include "traits.hpp"

#include <cmath>
#include <cstddef>
#include <functional> // For std::hash.
#include <limits>
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Pareto distribution. */
            struct dist_pareto;

            template <>
            struct is_continuous<dist_pareto>
            {
                typedef dist_pareto distribution_type;
                static constexpr bool value = true;
            };

            /** Pareto distribution. */
            struct dist_pareto
            {
                typedef dist_pareto type;
                typedef double result_type;

                static const std::string name;

            private:
                double m_alpha, m_x_min;
                double m_cache_expected_value, m_cache_variance, m_cache_standard_deviation;
                double m_cache_axa;

            public:
                /** Default constructor with unit alpha and unit minimum. */
                dist_pareto() noexcept
                    : m_alpha(1.0), m_x_min(1.0),
                    m_cache_expected_value(std::numeric_limits<double>::infinity()),
                    m_cache_variance(std::numeric_limits<double>::infinity()),
                    m_cache_standard_deviation(std::numeric_limits<double>::infinity()),
                    m_cache_axa(1.0)
                {
                }

                /** @brief Constructs a Pareto distribution from \p alpha and \p x_min.
                 *  @exception std::out_of_range \p alpha is not positive.
                 *  @exception std::out_of_range \p x_min is not positive.
                 */
                explicit dist_pareto(double alpha, double x_min)
                    : m_alpha(alpha), m_x_min(x_min),
                    m_cache_expected_value(alpha > 1.0 ? x_min * alpha / (alpha - 1.0) : std::numeric_limits<double>::infinity()),
                    m_cache_variance(alpha > 2.0 ? x_min * x_min / ((alpha - 1.0) * (alpha - 1.0) * (1.0 - 2.0 / alpha)) : std::numeric_limits<double>::infinity()),
                    m_cache_standard_deviation(alpha > 2.0 ? x_min / ((alpha - 1.0) * std::sqrt(1.0 - 2.0 / alpha)) : std::numeric_limits<double>::infinity()),
                    m_cache_axa(alpha * std::pow(x_min, alpha))
                {
                    if (alpha <= 0.0) throw std::out_of_range("<alpha> must be positive.");
                    if (x_min <= 0.0) throw std::out_of_range("<x_min> must be positive.");
                }

                //... to_std() const noexcept;

                /** Shape parameter of the distribution. */
                double alpha() const noexcept { return this->m_alpha; }
                /** Scale parameter of the distribution. */
                double x_min() const noexcept { return this->m_x_min; }

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
                double cdf(double x) const noexcept { return x < this->m_x_min ? 0.0 : (1.0 - std::pow(this->m_x_min / x, this->m_alpha)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return x < this->m_x_min ? 0.0 : this->m_cache_axa / std::pow(x, this->m_alpha + 1); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                double partial_moment(double a, double b) const noexcept
                {
                    if (b <= this->m_x_min) return 0.0;
                    if (a < this->m_x_min) a = this->m_x_min;
                    
                    // this->m_cache_axa = this->m_alpha * std::pow(this->m_x_min, this->m_alpha);
                    return this->m_cache_axa * (std::pow(b, t_nth_moment - this->m_alpha) - std::pow(a, t_nth_moment - this->m_alpha)) / (t_nth_moment - this->m_alpha);
                }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_alpha == other.m_alpha &&
                        this->m_x_min == other.m_x_min;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_pareto::name = "pareto";
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_pareto>
    {
        typedef ropufu::aftermath::probability::dist_pareto argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.alpha()) ^
                double_hash(x.x_min());
        }
    };
}

#endif // AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
