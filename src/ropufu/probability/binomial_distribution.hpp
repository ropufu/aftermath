
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED

#include "../number_traits.hpp"
#include "distribution_traits.hpp"

#include <cmath>       // std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::binomial_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval, std::swap

namespace ropufu::aftermath::probability
{
    /** Binomial distribution. */
    template <typename t_value_type = std::size_t, typename t_probability_type = double, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct binomial_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_discrete<binomial_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = binomial_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Binomial distribution.
     *  @todo Add tests!!
     */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct binomial_distribution
    {
        using type = binomial_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::binomial_distribution<t_value_type>;

        static constexpr char name[] = "binomial";

    private:
        value_type m_number_of_trials = 1;
        probability_type m_probability_of_success = 0;

        static constexpr void traits_check() noexcept
        {
            static_assert(std::numeric_limits<value_type>::is_integer, "Value type has to be an integer type.");
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

        void validate() const
        {
            if constexpr (std::numeric_limits<value_type>::is_signed)
            {
                if (this->m_number_of_trials < 0) throw std::logic_error("Number of trials cannot be negative.");
            } // if constexpr (...)
            if (this->m_number_of_trials == 0) throw std::logic_error("Number of trials cannot be zero.");

            if (!aftermath::is_finite(this->m_probability_of_success) || this->m_probability_of_success < 0 || this->m_probability_of_success > 1)
                throw std::logic_error("Probability must be a finite number between 0 and 1.");
        } // validate(...)

    public:
        /** Trivial case with one trial that always fails. */
        binomial_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ binomial_distribution(const std_type& distribution)
            : binomial_distribution(static_cast<value_type>(distribution.t()), static_cast<probability_type>(distribution.p()))
        {
        } // binomial_distribution(...)

        /** @brief Constructs a binomial distribution from the number of trials and probability of success.
         *  @exception std::logic_error \p probability_of_success is not inside the interval [0, 1].
         */
        explicit binomial_distribution(value_type number_of_trials, probability_type probability_of_success)
            : m_number_of_trials(number_of_trials), m_probability_of_success(probability_of_success)
        {
            type::traits_check();
            this->validate();
        } // binomial_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using t_type = decltype(std::declval<std_type>().t());
            using p_type = decltype(std::declval<std_type>().p());
            return std_type(static_cast<t_type>(this->m_number_of_trials), static_cast<p_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Number of trials in the experiment. */
        value_type t() const noexcept { return this->m_number_of_trials; }
        /** Number of trials in the experiment. */
        value_type number_of_trials() const noexcept { return this->m_number_of_trials; }
        /** Probability of success. */
        probability_type p() const noexcept { return this->m_probability_of_success; }
        /** Probability of success. */
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }
        /** Probability of failure. */
        probability_type probability_of_failure() const noexcept { return (1 - this->m_probability_of_success); }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept
        {
            expectation_type n = static_cast<expectation_type>(this->m_number_of_trials);
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return n * p;
        } // expected_value(...)
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type n = static_cast<expectation_type>(this->m_number_of_trials);
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return n * p * (1 - p);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }
        /** Smallest value in the distribution. */
        constexpr value_type min() const noexcept { return 0; }
        /** Largest value in the distribution. */
        value_type max() const noexcept { return this->m_number_of_trials; }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            if (k >= this->m_number_of_trials) return 1;
            
            probability_type p = 0;
            for (value_type j = 0; j <= k; ++j) p += this->pmf(j);
            return p;
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            expectation_type q = 1 - p;
            value_type n = this->m_number_of_trials;

            if (k > n || k < 0) return 0;
            if (k > n / 2)
            {
                k = n - k;
                std::swap(p, q);
            } // if (...)
            if (q == 0) return 0;
            value_type m = n - 2 * k;

            expectation_type result = static_cast<expectation_type>(scale);
            expectation_type v = p * q;
            expectation_type numerator = static_cast<expectation_type>(n - k);
            for (expectation_type i = 1; i <= k; ++i) result *= (++numerator) * v / i;
            for (value_type j = 0; j < m; ++j) result *= q;
            return static_cast<probability_type>(result);
        } // pmf(...)

        /** Support of the distribution. */
        std::vector<value_type> support() const
        {
            std::vector<value_type> result {};
            result.reserve(static_cast<std::size_t>(this->m_number_of_trials) + 1);
            for (value_type k = 0; k <= this->m_number_of_trials; ++k) result.push_back(k);
            return result;
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_number_of_trials == other.m_number_of_trials &&
                this->m_probability_of_success == other.m_probability_of_success;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }
    }; // struct binomial_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char binomial_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::binomial_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::binomial_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::value_type> value_hash = {};
            std::hash<typename argument_type::probability_type> probability_hash = {};
            return
                (value_hash(x.t()) << 4) ^ 
                (probability_hash(x.p()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
