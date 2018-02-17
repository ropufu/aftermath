
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::uniform_real_distribution

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Uniform (continuous) distribution. */
            template <typename t_result_type = double>
            struct dist_uniform;

            template <typename t_result_type>
            using dist_uniform_real = dist_uniform<t_result_type>;

            template <typename t_result_type>
            struct is_continuous<dist_uniform<t_result_type>>
            {
                using distribution_type = dist_uniform<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Uniform (continuous) distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_uniform
            {
                using type = dist_uniform<t_result_type>;
                using result_type = t_result_type;
                using param_type = t_result_type;
                using std_type = std::uniform_real_distribution<t_result_type>;

                static constexpr char name[] = "uniform";

            private:
                param_type m_a = 0;
                param_type m_b = 1;
                // ~~ Cached values ~~
                param_type m_cache_expected_value = static_cast<param_type>(0.5);
                param_type m_cache_variance = math_constants<result_type>::one_over_twelwe;
                param_type m_cache_standard_deviation = math_constants<result_type>::root_twelwe;
                param_type m_cache_length = 1;
                param_type m_cache_density = 1;

                void coerce() noexcept
                {
                    bool is_a_not_finite = std::isnan(this->m_a) || std::isinf(this->m_a);
                    bool is_b_not_finite = std::isnan(this->m_b) || std::isinf(this->m_b);
                    if (is_a_not_finite && is_b_not_finite)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Endpoints must be finite. Coerced to [0, 1].", __FUNCTION__, __LINE__);
                        this->m_a = 0;
                        this->m_b = 1;
                    }
                    else if (is_a_not_finite)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Left endpoint must be finite. Coerced to right - 1.", __FUNCTION__, __LINE__);
                        this->m_a = this->m_b - 1;
                    }
                    else if (is_b_not_finite)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Right endpoint must be finite. Coerced to left + 1.", __FUNCTION__, __LINE__);
                        this->m_a = this->m_a + 1;
                    }
                    param_type length = this->m_b - this->m_a; 
                    this->m_cache_expected_value = (this->m_a + this->m_b) / 2;
                    this->m_cache_variance = length * length / 12;
                    this->m_cache_standard_deviation = length / math_constants<result_type>::root_twelwe;
                    this->m_cache_length = length;
                    this->m_cache_density = 1 / length;
                } // coerce(...)

            public:
                /** Default constructor over the unit interval [0, 1]. */
                dist_uniform() noexcept { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_uniform(const std_type& distribution) noexcept : dist_uniform(static_cast<param_type>(distribution.a()), static_cast<param_type>(distribution.b())) { }

                /** @brief Constructs a uniform distribution over [\p a, \p b].
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p b does not exceed \p a.
                 */
                explicit dist_uniform(param_type a, param_type b) noexcept
                    : m_a(a), m_b(b)
                {
                    this->coerce();
                } // dist_uniform

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std_type(this->m_a, this->m_b);
                } // to_std(...)

                /** Left endpoint of the interval. */
                param_type min() const noexcept { return this->m_a; }
                /** Right endpoint of the interval. */
                param_type max() const noexcept { return this->m_b; }

                /** Expected value of the distribution. */
                param_type expected_value() const noexcept { return this->m_cache_expected_value; }
                /** Variance of the distribution. */
                param_type variance() const noexcept { return this->m_cache_variance; }
                /** Standard deviation of the distribution. */
                param_type standard_deviation() const noexcept { return this->m_cache_standard_deviation; }

                /** Expected value of the distribution. */
                param_type mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                param_type stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */
                param_type cdf(result_type x) const noexcept { return x < this->m_a ? 0 : (x >= this->m_b ? 1 : ((x - this->m_a) / this->m_cache_length)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                param_type pdf(result_type x) const noexcept { return x < this->m_a ? 0 : (x > this->m_b ? 0 : this->m_cache_density); }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_a == other.m_a &&
                        this->m_b == other.m_b;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                } // operator !=(...)
            }; // struct dist_uniform

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_uniform<t_result_type>::name[];
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_uniform<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_uniform<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                param_hash(x.min()) ^
                param_hash(x.max());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_UNIFORM_HPP_INCLUDED
