
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow, std::erfc, std::log
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::lognormal_distribution

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Lognormal distribution. */
            template <typename t_result_type = double>
            struct dist_lognormal;

            template <typename t_result_type>
            struct is_continuous<dist_lognormal<t_result_type>>
            {
                using distribution_type = dist_lognormal<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Lognormal distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_lognormal
            {
                using type = dist_lognormal<t_result_type>;
                using result_type = t_result_type;
                using param_type = t_result_type;
                using std_type = std::lognormal_distribution<t_result_type>;

                static constexpr char name[] = "lognorm"; // "lognormal"

            private:
                param_type m_mu = 0;
                param_type m_sigma = 1;
                // ~~ Cached values ~~
                param_type m_cache_expected_value = math_constants<result_type>::root_e;
                param_type m_cache_variance = (math_constants<result_type>::e - 1) * math_constants<result_type>::root_e;
                param_type m_cache_standard_deviation = (math_constants<result_type>::root_e_minus_one) * math_constants<result_type>::root_e;
                param_type m_cache_sigma_squared = 1;
                param_type m_cache_sigma_root_two = math_constants<result_type>::root_two;
                param_type m_cache_pdf_scale = math_constants<result_type>::one_div_root_two_pi;

                void coerce() noexcept
                {
                    if (std::isnan(this->m_mu) || std::isinf(this->m_mu))
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Mu must be finite. Coerced to 0.", __FUNCTION__, __LINE__);
                        this->m_mu = 0;
                    }
                    if (std::isnan(this->m_sigma) || std::isinf(this->m_sigma) || this->m_sigma <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Sigma must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_sigma = 1;
                    }
                    param_type v = this->m_sigma * this->m_sigma;

                    this->m_cache_expected_value = std::exp(this->m_mu + v / 2);
                    this->m_cache_variance = (std::exp(v) - 1) * std::exp(2 * this->m_mu + v);
                    this->m_cache_standard_deviation = std::sqrt(std::exp(v) - 1) * std::exp(this->m_mu + v / 2);
                    this->m_cache_sigma_squared = v;
                    this->m_cache_sigma_root_two = this->m_sigma * math_constants<result_type>::root_two; // sigma * std::sqrt(2)
                    this->m_cache_pdf_scale = math_constants<result_type>::one_div_root_two_pi / this->m_sigma; // 1 / (sigma * std::sqrt(2 pi))
                } // coerce(...)

            public:
                /** Default constructor with unit sigma. */
                dist_lognormal() noexcept { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_lognormal(const std_type& distribution) noexcept : dist_lognormal(static_cast<param_type>(distribution.m()), static_cast<param_type>(distribution.s())) { }

                /** @brief Constructs a lognormal distribution from mu and sigma, the mean and standard deviation of the corresponding normal distribution.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p sigma is not positive.
                 */
                explicit dist_lognormal(param_type mu, param_type sigma = 1) noexcept
                    : m_mu(mu), m_sigma(sigma)
                {
                    this->coerce();
                } // dist_lognormal(...)

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std_type(this->m_mu, this->m_sigma);
                } // to_std(...)

                /** Mean of the underlying normal distribution. */
                param_type mu() const noexcept { return this->m_mu; }
                /** Standard deviation of the underlying normal distribution. */
                param_type sigma() const noexcept { return this->m_sigma; }

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
                param_type cdf(result_type x) const noexcept { return std::erfc((this->m_mu - std::log(x)) / (this->m_cache_sigma_root_two)) / 2; }
                
                /** Probability density function (p.d.f.) of the distribution. */
                param_type pdf(result_type x) const noexcept
                {
                    return (this->m_cache_pdf_scale / x) * std::exp(-(std::log(x) - this->m_mu) * (std::log(x) - this->m_mu) / (2 * this->m_cache_variance));
                } // pdf(...)

                /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
                template <std::size_t t_nth_moment>
                param_type partial_moment(result_type a, result_type b) const noexcept
                {
                    return std::exp(t_nth_moment * this->m_mu + t_nth_moment * t_nth_moment * this->m_cache_sigma_squared / 2) * (
                        std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(a)) / (this->m_cache_sigma_root_two)) -
                        std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(b)) / (this->m_cache_sigma_root_two))) / 2;
                } // partial_moment(...)

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_mu == other.m_mu &&
                        this->m_sigma == other.m_sigma;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                } // operator !=(...)
            }; // struct dist_lognormal

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_lognormal<t_result_type>::name[];
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_lognormal<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_lognormal<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                param_hash(x.mu()) ^
                param_hash(x.sigma());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_LOGNORMAL_HPP_INCLUDED
