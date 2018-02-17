
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Negative Pareto distribution: a special case of scaled (0; x_max) Beta distribution with parameter beta = 1. */
            template <typename t_result_type = double>
            struct dist_negative_pareto;

            template <typename t_result_type>
            struct is_continuous<dist_negative_pareto<t_result_type>>
            {
                using distribution_type = dist_negative_pareto<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Negative Pareto distribution: special case of scaled (0; x_max) Beta distribution with parameter beta = 1.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_negative_pareto
            {
                using type = dist_negative_pareto<t_result_type>;
                using result_type = t_result_type;
                using param_type = t_result_type;
                using std_type = void;

                static constexpr char name[] = "negative_pareto"; // "negative pareto"

            private:
                param_type m_alpha = 1;
                param_type m_x_max = 1;
                // ~~ Cached values ~~
                param_type m_cache_expected_value = static_cast<param_type>(0.5);
                param_type m_cache_variance = math_constants<result_type>::one_over_twelwe;
                param_type m_cache_standard_deviation = math_constants<result_type>::one_over_root_twelwe;
                param_type m_cache_axa = 1;

                void coerce() noexcept
                {
                    if (std::isnan(this->m_alpha) || std::isinf(this->m_alpha) || this->m_alpha <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Shape must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_alpha = 1;
                    }
                    if (std::isnan(this->m_x_max) || std::isinf(this->m_x_max) || this->m_x_max <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Scale must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_x_max = 1;
                    }
                    this->m_cache_expected_value = this->m_x_max * this->m_alpha / (this->m_alpha + 1);
                    this->m_cache_variance = this->m_x_max * this->m_x_max / ((this->m_alpha + 1) * (this->m_alpha + 1) * (1 + 2 / this->m_alpha));
                    this->m_cache_standard_deviation = this->m_x_max / ((this->m_alpha + 1) * std::sqrt(1 + 2 / this->m_alpha));
                    this->m_cache_axa = this->m_alpha / std::pow(this->m_x_max, this->m_alpha);
                } // coerce(...)

            public:
                /** Default constructor with unit alpha and unit maximum. */
                dist_negative_pareto() noexcept { }

                // /** Constructor and implicit conversion from standard distribution. */
                // dist_negative_pareto(const std_type& distribution) noexcept : dist_negative_pareto(static_cast<param_type>(distribution.alpha()), static_cast<param_type>(distribution.x_max())) { }

                /** @brief Constructs a negative Pareto distribution from \p alpha and \p x_max.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p alpha is not positive.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p x_max is not positive.
                 */
                explicit dist_negative_pareto(param_type alpha, param_type x_max) noexcept
                    : m_alpha(alpha), m_x_max(x_max)
                {
                    this->coerce();
                } // dist_negative_pareto(...)

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    quiet_error::instance().push(not_an_error::all_good, severity_level::not_at_all, "Negative pareto distribution has not been implemented yet.", __FUNCTION__, __LINE__);
                    return; // std_type(this->m_alpha, this->m_x_max);
                }

                /** Shape parameter of the distribution. */
                param_type alpha() const noexcept { return this->m_alpha; }
                /** Scale parameter of the distribution. */
                param_type x_max() const noexcept { return this->m_x_max; }

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
                param_type cdf(result_type x) const noexcept { return x <= 0 ? 0 : (x >= this->m_x_max ? 1 : std::pow(x / this->m_x_max, this->m_alpha)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                param_type pdf(result_type x) const noexcept { return (x <= 0 || x >= this->m_x_max) ? 0 : (this->m_cache_axa * std::pow(x, this->m_alpha - 1)); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                param_type partial_moment(result_type a, result_type b) const noexcept
                {
                    if (b <= 0 || a >= this->m_x_max) return 0;
                    if (b > this->m_x_max) b = this->m_x_max;
                    if (a < 0) a = 0;
                
                    // this->m_cache_axa = this->m_xalpha / std::pow(this->m_x_max, this->m_alpha);
                    return this->m_cache_axa * (std::pow(b, t_nth_moment + this->m_alpha) - std::pow(a, t_nth_moment + this->m_alpha)) / (t_nth_moment + this->m_alpha);
                } // partial_moment(...)

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_alpha == other.m_alpha &&
                        this->m_x_max == other.m_x_max;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                } // operator !=(...)
            }; // struct dist_negative_pareto

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_negative_pareto<t_result_type>::name[];
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_negative_pareto<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_negative_pareto<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                param_hash(x.alpha()) ^
                param_hash(x.x_max());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_NEGATIVE_PARETO_HPP_INCLUDED
