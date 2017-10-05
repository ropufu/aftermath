
#ifndef AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
#define AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED

#include "../not_an_error.hpp"
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
                using distribution_type = dist_pareto;
                static constexpr bool value = true;
            };

            /** @brief Pareto distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            struct dist_pareto
            {
                using type = dist_pareto;
                using result_type = double;
                using std_type = void;

                static const std::string name;

            private:
                double m_alpha, m_x_min;
                double m_cache_expected_value, m_cache_variance, m_cache_standard_deviation;
                double m_cache_axa;

            public:
                /** Default constructor with unit alpha and unit minimum. */
                dist_pareto() noexcept : dist_pareto(1, 1) { }

                // /** Constructor and implicit conversion from standard distribution. */
                // dist_pareto(const std_type& distribution) noexcept : dist_pareto(distribution.alpha(), distribution.x_min()) { }

                /** @brief Constructs a Pareto distribution from \p alpha and \p x_min.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p alpha is not positive.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p x_min is not positive.
                 */
                explicit dist_pareto(double alpha, double x_min) noexcept
                    : m_alpha(alpha), m_x_min(x_min),
                    m_cache_expected_value(alpha > 1.0 ? x_min * alpha / (alpha - 1.0) : std::numeric_limits<double>::infinity()),
                    m_cache_variance(alpha > 2.0 ? x_min * x_min / ((alpha - 1.0) * (alpha - 1.0) * (1.0 - 2.0 / alpha)) : std::numeric_limits<double>::infinity()),
                    m_cache_standard_deviation(alpha > 2.0 ? x_min / ((alpha - 1.0) * std::sqrt(1.0 - 2.0 / alpha)) : std::numeric_limits<double>::infinity()),
                    m_cache_axa(alpha * std::pow(x_min, alpha))
                {
                    if (alpha <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "<alpha> must be positive.", __FUNCTION__, __LINE__);
                    }
                    if (x_min <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "<x_min> must be positive.", __FUNCTION__, __LINE__);
                    }
                }

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    quiet_error::instance().push(not_an_error::all_good, severity_level::not_at_all, "Negative pareto distribution has not been implemented yet.", __FUNCTION__, __LINE__);
                    return; // std_type(this->m_alpha, this->m_x_max);
                }

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
                double cdf(double x) const noexcept { return x < this->m_x_min ? 0 : (1 - std::pow(this->m_x_min / x, this->m_alpha)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return x < this->m_x_min ? 0 : this->m_cache_axa / std::pow(x, this->m_alpha + 1); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                double partial_moment(double a, double b) const noexcept
                {
                    if (b <= this->m_x_min) return 0;
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
        using argument_type = ropufu::aftermath::probability::dist_pareto;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.alpha()) ^
                double_hash(x.x_min());
        }
    };
}

#endif // AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
