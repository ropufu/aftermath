
#ifndef AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
#define AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <limits>     // std::numeric_limits

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Pareto distribution. */
            template <typename t_result_type = double>
            struct dist_pareto;

            template <typename t_result_type>
            struct is_continuous<dist_pareto<t_result_type>>
            {
                using distribution_type = dist_pareto<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Pareto distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_pareto
            {
                using type = dist_pareto<t_result_type>;
                using result_type = t_result_type;
                using param_type = t_result_type;
                using std_type = void;

                static constexpr char name[] = "pareto";

                void coerce() noexcept
                {
                    if (std::isnan(this->m_alpha) || std::isinf(this->m_alpha) || this->m_alpha <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Shape must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_alpha = 1;
                    }
                    if (std::isnan(this->m_x_min) || std::isinf(this->m_x_min) || this->m_x_min <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Scale must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_x_min = 1;
                    }
                    this->m_cache_expected_value = std::numeric_limits<param_type>::infinity();
                    this->m_cache_variance = std::numeric_limits<param_type>::infinity();
                    this->m_cache_standard_deviation = std::numeric_limits<param_type>::infinity();

                    if (this->m_alpha > 1) this->m_cache_expected_value = this->m_x_min * this->m_alpha / (this->m_alpha - 1);
                    if (this->m_alpha > 2) this->m_cache_variance = this->m_x_min * this->m_x_min / ((this->m_alpha - 1) * (this->m_alpha - 1) * (1 - 2 / this->m_alpha));
                    if (this->m_alpha > 2) this->m_cache_standard_deviation = this->m_x_min / ((this->m_alpha - 1) * std::sqrt(1 - 2 / this->m_alpha));
                    this->m_cache_axa = this->m_alpha * std::pow(this->m_x_min, this->m_alpha);
                } // coerce(...)

            private:
                param_type m_alpha = 1;
                param_type m_x_min = 1;
                // ~~ Cached values ~~
                param_type m_cache_expected_value = std::numeric_limits<param_type>::infinity();
                param_type m_cache_variance = std::numeric_limits<param_type>::infinity();
                param_type m_cache_standard_deviation = std::numeric_limits<param_type>::infinity();
                param_type m_cache_axa = 1;

            public:
                /** Default constructor with unit alpha and unit minimum. */
                dist_pareto() noexcept { }

                // /** Constructor and implicit conversion from standard distribution. */
                // dist_pareto(const std_type& distribution) noexcept : dist_pareto(static_cast<param_type>(distribution.alpha()), static_cast<param_type>(distribution.x_min())) { }

                /** @brief Constructs a Pareto distribution from \p alpha and \p x_min.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p alpha is not positive.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p x_min is not positive.
                 */
                explicit dist_pareto(param_type alpha, param_type x_min) noexcept
                    : m_alpha(alpha), m_x_min(x_min)
                {
                    this->coerce();
                } // dist_pareto(...)

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    quiet_error::instance().push(not_an_error::all_good, severity_level::not_at_all, "Negative pareto distribution has not been implemented yet.", __FUNCTION__, __LINE__);
                    return; // std_type(this->m_alpha, this->m_x_min);
                }

                /** Shape parameter of the distribution. */
                param_type alpha() const noexcept { return this->m_alpha; }
                /** Scale parameter of the distribution. */
                param_type x_min() const noexcept { return this->m_x_min; }

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
                param_type cdf(result_type x) const noexcept { return x < this->m_x_min ? 0 : (1 - std::pow(this->m_x_min / x, this->m_alpha)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                param_type pdf(result_type x) const noexcept { return x < this->m_x_min ? 0 : this->m_cache_axa / std::pow(x, this->m_alpha + 1); }

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                param_type partial_moment(result_type a, result_type b) const noexcept
                {
                    if (b <= this->m_x_min) return 0;
                    if (a < this->m_x_min) a = this->m_x_min;
                    
                    // this->m_cache_axa = this->m_alpha * std::pow(this->m_x_min, this->m_alpha);
                    return this->m_cache_axa * (std::pow(b, t_nth_moment - this->m_alpha) - std::pow(a, t_nth_moment - this->m_alpha)) / (t_nth_moment - this->m_alpha);
                } // partial_moment(...)

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_alpha == other.m_alpha &&
                        this->m_x_min == other.m_x_min;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                } // operator !=(...)
            }; // struct dist_pareto

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_pareto<t_result_type>::name[];
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_pareto<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_pareto<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                param_hash(x.alpha()) ^
                param_hash(x.x_min());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // AFTERMATH_PROBABILITY_DIST_PARETO_HPP_INCLUDED
