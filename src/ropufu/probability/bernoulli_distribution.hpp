
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED

#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <limits>     // std::numeric_limits::is_integer
#include <random>     // std::bernoulli_distribution
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval, std::swap

namespace ropufu::aftermath::probability
{
    /** Binomial distribution. */
    template <typename t_probability_type = double, typename t_expectation_type = t_probability_type>
    struct bernoulli_distribution;

    template <typename t_probability_type, typename t_expectation_type>
    struct is_discrete<bernoulli_distribution<t_probability_type, t_expectation_type>>
    {
        using distribution_type = bernoulli_distribution<t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Binomial distribution. */
    template <typename t_probability_type, typename t_expectation_type>
    struct bernoulli_distribution
    {
        using type = bernoulli_distribution<t_probability_type, t_expectation_type>;
        using value_type = bool;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::bernoulli_distribution;

        static constexpr char name[] = "bernoulli";

    private:
        probability_type m_probability_of_success = 0;
        // ~~ Cached values ~~
        probability_type m_cache_probability_of_failure = 1;
        expectation_type m_cache_expected_value = 0;
        expectation_type m_cache_standard_deviation = 0;
        expectation_type m_cache_variance = 0;

        static constexpr void traits_check()
        {
        } // traits_check(...)

        bool validate(std::error_code& ec) const noexcept
        {
            if (std::isnan(this->m_probability_of_success) || std::isinf(this->m_probability_of_success) || this->m_probability_of_success < 0 || this->m_probability_of_success > 1)
                return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Probability of success must be in the range from 0 to 1.", false);
            return true;
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_probability_of_failure = 1 - this->m_probability_of_success;
            this->m_cache_expected_value = static_cast<expectation_type>(this->m_probability_of_success);
            this->m_cache_variance = static_cast<expectation_type>(this->m_probability_of_success * this->m_cache_probability_of_failure);
            this->m_cache_standard_deviation = static_cast<expectation_type>(std::sqrt(this->m_cache_variance));
        } // reset(...)

    public:
        /** Default constructor with one trial and probability of success 1/2. */
        bernoulli_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ bernoulli_distribution(const std_type& distribution) noexcept
            : m_probability_of_success(static_cast<probability_type>(distribution.p()))
        {
            type::traits_check();
            this->cahce();
        } // bernoulli_distribution(...)

        /** @brief Constructs a binomial distribution from the number of trials, \p n, and probability of success.
         *  @param ec Set to std::errc::invalid_argument if \p probability_of_success is not in the interval [0, 1].
         *  @param ec Set to std::errc::invalid_argument if \p number_of_trials is not in the interval [0, 1].
         */
        explicit bernoulli_distribution(probability_type probability_of_success, std::error_code& ec) noexcept
            : m_probability_of_success(probability_of_success)
        {
            type::traits_check();
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_probability_of_success = 0;
            } // if (...)
        } // bernoulli_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_number_of_trials, static_cast<typename std_type::param_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Probability of success. */
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }
        /** Probability of failure. */
        probability_type probability_of_failure() const noexcept { return this->m_cache_probability_of_failure; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_cache_expected_value; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_cache_standard_deviation; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            return k ? 1 : this->m_cache_probability_of_failure;
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k) const noexcept
        {
            return k ? this->m_probability_of_success : this->m_cache_probability_of_failure;
        } // pdf(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_probability_of_success == other.m_probability_of_success;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }
    }; // struct bernoulli_distribution

    // ~~ Definitions ~~
    template <typename t_probability_type, typename t_expectation_type>
    constexpr char bernoulli_distribution<t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::bernoulli_distribution<t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::bernoulli_distribution<t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::probability_type> probability_hash = {};

            return
                probability_hash(x.probability_of_success());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
