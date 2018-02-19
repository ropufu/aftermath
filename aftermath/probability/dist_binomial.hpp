
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::binomial_distribution

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Binomial distribution. */
            template <typename t_result_type = std::size_t, typename t_param_type = double>
            struct dist_binomial;

            template <typename t_result_type, typename t_param_type>
            struct is_discrete<dist_binomial<t_result_type, t_param_type>>
            {
                using distribution_type = dist_binomial<t_result_type, t_param_type>;
                static constexpr bool value = true;
            }; // struct is_discrete

            /** @brief Binomial distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type, typename t_param_type>
            struct dist_binomial
            {
                using type = dist_binomial<t_result_type, t_param_type>;
                using result_type = t_result_type;
                using param_type = t_param_type;
                using std_type = std::binomial_distribution<t_result_type>;

                static constexpr char name[] = "binomial";

            private:
                result_type m_number_of_trials = 1;
                param_type m_probability_of_success = 0;
                // ~~ Cached values ~~
                param_type m_cache_probability_of_failure = 1;
                param_type m_cache_expected_value = 0;
                param_type m_cache_standard_deviation = 0;
                param_type m_cache_variance = 0;

                void coerce() noexcept
                {
                    if (this->m_number_of_trials <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Number of trials must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_number_of_trials = 1;
                    }
                    if (std::isnan(this->m_probability_of_success) || std::isinf(this->m_probability_of_success) || this->m_probability_of_success < 0 || this->m_probability_of_success > 1)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Probability of success must be in the range from 0 to 1. Coerced to [0, 1].", __FUNCTION__, __LINE__);
                        if (this->m_probability_of_success < 0) this->m_probability_of_success = 0;
                        if (this->m_probability_of_success > 1) this->m_probability_of_success = 1;
                    }
                    this->m_cache_probability_of_failure = 1 - this->m_probability_of_success;
                    this->m_cache_expected_value = this->m_number_of_trials * this->m_probability_of_success;
                    this->m_cache_variance = this->m_cache_expected_value * this->m_cache_probability_of_failure;
                    this->m_cache_standard_deviation = std::sqrt(m_cache_variance);
                } // coerce(...)

            public:
                /** Default constructor with one trial and probability of success 1/2. */
                dist_binomial() noexcept { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_binomial(const std_type& distribution) noexcept : dist_binomial(distribution.t(), static_cast<param_type>(distribution.p())) { }

                /** @brief Constructs a binomial distribution from the number of trials, \p n, and probability of success.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p probability_of_success is not in the interval [0, 1].
                 */
                explicit dist_binomial(result_type number_of_trials, param_type probability_of_success) noexcept
                    : m_number_of_trials(number_of_trials), m_probability_of_success(probability_of_success)
                {
                    this->coerce();
                } // dist_binomial(...)

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std_type(this->m_number_of_trials, static_cast<typename std_type::param_type>(this->m_probability_of_success));
                } // to_std(...)

                /** Number of trials in the experiment. */
                result_type number_of_trials() const noexcept { return this->m_number_of_trials; }
                /** Probability of success. */
                param_type probability_of_success() const noexcept { return this->m_probability_of_success; }
                /** Probability of failure. */
                param_type probability_of_failure() const noexcept { return this->m_cache_probability_of_failure; }

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
                param_type cdf(result_type k) const noexcept
                {
                    if (k >= this->m_number_of_trials) return 1;
                    
                    param_type p = 0;
                    for (result_type j = 0; j <= k; ++j) p += this->pdf(j);
                    return p;
                } // cdf(...)

                /** Point mass function (p.m.f.) of the distribution. */
                param_type pdf(result_type k) const noexcept
                {
                    return type::n_choose_k(this->m_number_of_trials, k) * static_cast<param_type>(std::pow(this->m_probability_of_success, k) * std::pow(this->m_cache_probability_of_failure, this->m_number_of_trials - k));
                } // pdf(...)

                /** Binomial coefficient. */
                static param_type n_choose_k(result_type n, result_type k) noexcept
                {
                    if (k > n) return 0;
                    if (k > n / 2) k = n - k;

                    param_type result = 1;
                    result_type numerator = n - k;
                    for (result_type i = 1; i <= k; ++i) result *= static_cast<param_type>(++numerator) / i;
                    return result;
                } // n_choose_k(...)

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return
                        this->m_number_of_trials == other.m_number_of_trials &&
                        this->m_probability_of_success == other.m_probability_of_success;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }
            }; // struct dist_binomial

            // ~~ Definitions ~~
            template <typename t_result_type, typename t_param_type> constexpr char dist_binomial<t_result_type, t_param_type>::name[];
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type, typename t_param_type>
    struct hash<ropufu::aftermath::probability::dist_binomial<t_result_type, t_param_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_binomial<t_result_type, t_param_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::result_type> result_hash = { };
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                result_hash(x.number_of_trials()) ^ 
                param_hash(x.probability_of_success());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_BINOMIAL_HPP_INCLUDED
