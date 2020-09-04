
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED

#include "../number_traits.hpp"
#include "concepts.hpp"

#include <cmath>       // std::sqrt
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::bernoulli_distribution
#include <stdexcept>   // std::logic_error
#include <string_view> // std::string_view
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval
#include <vector>      // std::vector

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#define ROPUFU_TMP_TYPENAME bernoulli_distribution<t_probability_type, t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** Bernoulli distribution. */
    template <std::floating_point t_probability_type = double,
        std::floating_point t_expectation_type = t_probability_type>
    struct bernoulli_distribution;

    template <std::floating_point t_probability_type, std::floating_point t_expectation_type>
    struct is_discrete<ROPUFU_TMP_TYPENAME>
    {
        using distribution_type = ROPUFU_TMP_TYPENAME;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Bernoulli distribution.
     *  @todo Add tests!!
     */
    template <std::floating_point t_probability_type, std::floating_point t_expectation_type>
    struct bernoulli_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = bool;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::bernoulli_distribution;

        static constexpr std::string_view name = "bernoulli";

    private:
        probability_type m_probability_of_success = 0;

        void validate() const
        {
            if (!aftermath::is_finite(this->m_probability_of_success) || this->m_probability_of_success < 0 || this->m_probability_of_success > 1)
                throw std::logic_error("Probability must be a finite number between 0 and 1.");
        } // validate(...)

    public:
        /** Trivial case when trials always fail. */
        bernoulli_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ bernoulli_distribution(const std_type& distribution)
            : bernoulli_distribution(static_cast<probability_type>(distribution.p()))
        {
        } // bernoulli_distribution(...)

        /** @exception std::logic_error \p probability_of_success is not inside the interval [0, 1]. */
        explicit bernoulli_distribution(probability_type probability_of_success)
            : m_probability_of_success(probability_of_success)
        {
            this->validate();
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
        probability_type probability_of_failure() const noexcept { return (1 - this->m_probability_of_success); }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return static_cast<expectation_type>(this->m_probability_of_success); }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return p * (1 - p);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            return k ? 1 : (1 - this->m_probability_of_success);
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            return k ?
                (scale * this->m_probability_of_success) :
                (scale - scale * this->m_probability_of_success);
        } // pmf(...)

        /** Support of the distribution. */
        std::vector<value_type> support() const noexcept
        {
            return {false, true};
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_probability_of_success == other.m_probability_of_success;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }
    }; // struct bernoulli_distribution
} // namespace ropufu::aftermath::probability

namespace std
{
    template <std::floating_point t_probability_type, std::floating_point t_expectation_type>
    struct hash<ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME>
    {
        using argument_type = ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::probability_type> probability_hash = {};
            return
                (probability_hash(x.p()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
