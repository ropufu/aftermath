
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED

#include "../number_traits.hpp"
#include "distribution_traits.hpp"

#include <cmath>       // std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::bernoulli_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval

namespace ropufu::aftermath::probability
{
    /** Bernoulli distribution. */
    template <typename t_probability_type = double, typename t_expectation_type = t_probability_type>
    struct bernoulli_distribution;

    template <typename t_probability_type, typename t_expectation_type>
    struct is_discrete<bernoulli_distribution<t_probability_type, t_expectation_type>>
    {
        using distribution_type = bernoulli_distribution<t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Bernoulli distribution.
     *  @todo Add tests!!
     */
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

        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

        void validate() const
        {
            if (!aftermath::is_finite(this->m_probability_of_success) || this->m_probability_of_success < 0 || this->m_probability_of_success > 1)
                throw std::logic_error("Probability must be a finite number between 0 and 1.");
        } // validate(...)

        void cahce() noexcept
        {
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);

            this->m_cache_probability_of_failure = 1 - this->m_probability_of_success;
            this->m_cache_expected_value = p;
            this->m_cache_variance = p * (1 - p);
            this->m_cache_standard_deviation = std::sqrt(this->m_cache_variance);
        } // cahce(...)

    public:
        /** Trivial case when trials always fail. */
        bernoulli_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ bernoulli_distribution(const std_type& distribution)
            : bernoulli_distribution(static_cast<probability_type>(distribution.p()))
        {
        } // bernoulli_distribution(...)

        /** @exception std::logic_error \p probability_of_success is not inside the interval [0, 1]. */
        explicit bernoulli_distribution(probability_type probability_of_success)
            : m_probability_of_success(probability_of_success)
        {
            type::traits_check();

            this->validate();
            this->cahce();
        } // bernoulli_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using p_type = decltype(std::declval<std_type>().p());
            return std_type(static_cast<p_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Probability of success. */
        probability_type p() const noexcept { return this->m_probability_of_success; }
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
        } // pmf(...)

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
                (probability_hash(x.p()));
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
