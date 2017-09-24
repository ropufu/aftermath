
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED

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
            /** Uniform (continuous) distribution. */
            struct dist_uniform;

            template <>
            struct is_continuous<dist_uniform>
            {
                typedef dist_uniform distribution_type;
                static constexpr bool value = true;
            };

            /** Uniform (continuous) distribution. */
            struct dist_uniform
            {
                typedef dist_uniform type;
                typedef double result_type;

                static const std::string name;

            private:
                double m_a, m_b;
                double m_cache_expected_value, m_cache_variance, m_cache_standard_deviation;
                double m_cache_length, m_cache_density;

            public:
                /** Default constructor over the unit interval [0, 1]. */
                dist_uniform() noexcept
                    : m_a(0.0), m_b(1.0),
                    m_cache_expected_value(0.5),
                    m_cache_variance(math_constants::one_over_twelwe),
                    m_cache_standard_deviation(math_constants::one_over_root_twelwe),
                    m_cache_length(1.0), m_cache_density(1.0)
                {
                }

                /** @brief Constructs a uniform distribution over [\p a, \p b].
                 *  @exception std::logic_error \p b does not exceed \p a.
                 */
                explicit dist_uniform(double a = 0.0, double b = 1.0)
                    : m_a(a), m_b(b),
                    m_cache_expected_value((a + b) / 2), 
                    m_cache_variance((b - a) * (b - a) / 12), 
                    m_cache_standard_deviation((b - a) / math_constants::root_twelwe),
                    m_cache_length(b - a), m_cache_density(1 / (b - a))
                {
                    if (a >= b) throw std::logic_error("<b> must be greater than <a>.");
                }

                std::uniform_real_distribution<double> to_std() const noexcept
                {
                    return std::uniform_real_distribution<double>(this->m_a, this->m_b);
                }

                /** Left endpoint of the interval. */
                double min() const noexcept { return this->m_a; }
                /** Right endpoint of the interval. */
                double max() const noexcept { return this->m_b; }

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
                double cdf(double x) const noexcept { return x < this->m_a ? 0.0 : (x >= this->m_b ? 1.0 : ((x - this->m_a) / this->m_cache_length)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                double pdf(double x) const noexcept { return x < this->m_a ? 0.0 : (x > this->m_b ? 0.0 : this->m_cache_density); }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_a == other.m_a &&
                        this->m_b == other.m_b;
                }

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                }
            };
            
            /** Name of the distribution. */
            const std::string dist_uniform::name = "uniform";
        }
    }
}

namespace std
{
    template <>
    struct hash<ropufu::aftermath::probability::dist_uniform>
    {
        typedef ropufu::aftermath::probability::dist_uniform argument_type;
        typedef std::size_t result_type;

        result_type operator()(argument_type const& x) const noexcept
        {
            std::hash<double> double_hash = {};
            return
                double_hash(x.min()) ^
                double_hash(x.max());
        }
    };
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED
