
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::exp
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::exponential_distribution

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Exponential distribution. */
            template <typename t_result_type = double>
            struct dist_exponential;

            template <typename t_result_type>
            struct is_continuous<dist_exponential<t_result_type>>
            {
                using distribution_type = dist_exponential<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Exponential distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_exponential
            {
                using type = dist_exponential<t_result_type>;
                using result_type = t_result_type;
                using param_type = t_result_type;
                using std_type = std::exponential_distribution<t_result_type>;

                static constexpr char name[] = "exp"; // "exponential"
                static const type standard;

            private:
                param_type m_lambda = 1;
                // ~~ Cached values ~~
                param_type m_cache_mu = 1;
                param_type m_cache_variance = 1;

                void coerce() noexcept
                {
                    if (std::isnan(this->m_lambda) || std::isinf(this->m_lambda) || this->m_lambda <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Rate must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_lambda = 1;
                    }
                    this->m_cache_mu = 1 / this->m_lambda;
                    this->m_cache_variance = this->m_cache_mu * this->m_cache_mu;
                } // coerce(...)

            public:
                /** Default constructor with expected value 1. */
                dist_exponential() noexcept { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_exponential(const std_type& distribution) noexcept : dist_exponential(static_cast<param_type>(distribution.lambda())) { }

                /** @brief Constructs an exponential distribution from the expected value.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p rate_of_events is not positive.
                 */
                explicit dist_exponential(param_type rate_of_events) noexcept
                    : m_lambda(rate_of_events)
                {
                    this->coerce();
                } // dist_exponential(...)

                /** Converts the distribution to its standard built-in counterpart. */
                std_type to_std() const noexcept
                {
                    return std_type(this->m_lambda);
                } // to_std(...)
                
                /** Rate of the exponential distribution. */
                param_type rate() const noexcept { return this->m_lambda; }

                /** Expected value of the distribution. */
                param_type expected_value() const noexcept { return this->m_cache_mu; }
                /** Variance of the distribution. */
                param_type variance() const noexcept { return this->m_cache_variance; }
                /** Standard deviation of the distribution. */
                param_type standard_deviation() const noexcept { return this->m_cache_mu; }

                /** Expected value of the distribution. */
                param_type mean() const noexcept { return this->expected_value(); }
                /** Standard deviation of the distribution. */
                param_type stddev() const noexcept { return this->standard_deviation(); }

                /** Cumulative distribution function (c.d.f.) of the distribution. */                
                param_type cdf(result_type x) const noexcept { return x < 0 ? 0 : (1 - std::exp(-this->m_lambda * x)); }
                
                /** Probability density function (p.d.f.) of the distribution. */
                param_type pdf(result_type x) const noexcept { return x < 0 ? 0 : (this->m_lambda * std::exp(-this->m_lambda * x)); }

                /** Checks if the two distributions are the same. */
                bool operator ==(const type& other) const noexcept
                {
                    return this->m_lambda == other.m_lambda;
                } // operator ==(...)

                /** Checks if the two distributions are different. */
                bool operator !=(const type& other) const noexcept
                {
                    return !this->operator ==(other);
                } // operator !=(...)
            }; // struct dist_exponential

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_exponential<t_result_type>::name[];
            template <typename t_result_type>
            const dist_exponential<t_result_type> dist_exponential<t_result_type>::standard = dist_exponential<t_result_type>(1);
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_exponential<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_exponential<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return 
                param_hash(x.rate());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_EXPONENTIAL_HPP_INCLUDED
