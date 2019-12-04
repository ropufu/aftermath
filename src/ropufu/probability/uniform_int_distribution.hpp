
#ifndef ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED

#include "../number_traits.hpp"
#include "../math_constants.hpp"
#include "distribution_traits.hpp"

#include <cmath>       // std::sqrt
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::uniform_int_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval
#include <vector>      // std::vector

namespace ropufu::aftermath::probability
{
    /** Discrete uniform distribution. */
    template <typename t_value_type = std::size_t, typename t_probability_type = double, typename t_expectation_type = t_probability_type>
    struct uniform_int_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_discrete<uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Discrete uniform distribution.
     *  @todo Add tests!!
     */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct uniform_int_distribution
    {
        using type = uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::uniform_int_distribution<value_type>;

        static constexpr char name[] = "uniform_int";

    private:
        value_type m_min = 0;
        value_type m_max = 1;

        static constexpr void traits_check() noexcept
        {
            static_assert(std::numeric_limits<value_type>::is_integer, "Value type has to be an integer type.");
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

        void validate() const
        {
            if (this->m_min > this->m_max)
                throw std::logic_error("Smallest value cannot exceed than the largest value.");
        } // validate(...)

    public:
        /** Trivial case when trials always fail. */
        uniform_int_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ uniform_int_distribution(const std_type& distribution)
            : uniform_int_distribution(distribution.a(), distribution.b())
        {
        } // uniform_int_distribution(...)

        /** @exception std::logic_error \p min exceeds \p max. */
        explicit uniform_int_distribution(value_type min, value_type max)
            : m_min(min), m_max(max)
        {
            type::traits_check();
            this->validate();
        } // uniform_int_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_min, this->m_max);
        } // to_std(...)

        /** Smallest value. */
        value_type a() const noexcept { return this->m_min; }
        /** Largest value. */
        value_type b() const noexcept { return this->m_max; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept
        {
            expectation_type x = static_cast<expectation_type>(this->m_min) / 2;
            expectation_type y = static_cast<expectation_type>(this->m_max) / 2;
            return x + y;
        } // expected_value(...)
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type diam = static_cast<expectation_type>(this->m_max - this->m_min);
            return math_constants<expectation_type>::one_over_twelwe * diam * (diam + 2);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }
        /** Smallest value in the distribution. */
        value_type min() const noexcept { return this->m_min; }
        /** Largest value in the distribution. */
        value_type max() const noexcept { return this->m_max; }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            if (k < this->m_min) return 0;
            if (k >= this->m_max) return 1;
            k -= this->m_min;
            probability_type diam = static_cast<probability_type>(this->m_max - this->m_min);
            return static_cast<probability_type>(k + 1) / (diam + 1);
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            if (k < this->m_min) return 0;
            if (k > this->m_max) return 0;
            probability_type diam = static_cast<probability_type>(this->m_max - this->m_min);
            return scale / (diam + 1);
        } // pmf(...)

        /** @brief Support of the distribution.
         *  @exception std::overflow_error Support too large to be stored in memory.
         */
        std::vector<value_type> support() const
        {
            std::size_t diam = static_cast<std::size_t>(this->m_max - this->m_min);
            if (diam + 1 < diam) throw std::overflow_error("Support too large to be stored in memory.");
            std::vector<value_type> result {};
            result.reserve(diam + 1);
            for (value_type k = this->m_min; k <= this->m_max; ++k) result.push_back(k);
            return result;
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_min == other.m_min &&
                this->m_max == other.m_max;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }
    }; // struct uniform_int_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::value_type> value_hash = {};
            return
                (value_hash(x.a()) << 4) ^ 
                (value_hash(x.b()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED
