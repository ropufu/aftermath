
#ifndef ROPUFU_AFTERMATH_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../number_traits.hpp"
#include "distribution_traits.hpp"

#include <cmath>       // std::sqrt, std::exp, std::erfc
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::normal_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval

namespace ropufu::aftermath::probability
{
    /** Normal (Gaussian) distribution with zero mean and unit variance. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct standard_normal_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct has_right_tail<standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct has_right_tail

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct has_left_tail<standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct has_left_tail

    /** Normal (Gaussian) distribution with zero mean and unit variance. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct standard_normal_distribution
    {
        using type = standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::normal_distribution<t_value_type>;

        static constexpr char name[] = "std norm";

    private:
        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_floating_point_v<value_type>, "Value type has to be a floating point type.");
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

    public:
        /** Default constructor with zero mean and unit variance. */
        constexpr standard_normal_distribution() noexcept { type::traits_check(); }

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(0, 1);
        } // to_std(...)

        /** Mean of the distribution. */
        constexpr expectation_type mu() const noexcept { return 0; }
        /** Standard deviation of the distribution. */
        constexpr expectation_type sigma() const noexcept { return 1; }

        /** Expected value of the distribution. */
        constexpr expectation_type expected_value() const noexcept { return 0; }
        /** Variance of the distribution. */
        constexpr expectation_type variance() const noexcept { return 1; }
        /** Standard deviation of the distribution. */
        constexpr expectation_type standard_deviation() const noexcept { return 1; }

        /** Expected value of the distribution. */
        constexpr expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        constexpr expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type x) const noexcept
        {
            return static_cast<probability_type>(0.5) *
                static_cast<probability_type>(std::erfc(-math_constants<expectation_type>::one_div_root_two * x));
        } // cdf(...)

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return math_constants<expectation_type>::one_div_root_two_pi * static_cast<expectation_type>(std::exp(-math_constants<value_type>::one_half * x * x));
        } // pdf(...)
        
        /** @brief For a given 0 <= p <= 1, find t such that cdf(t) = p.
         *  @remark Solving cdf(t) = p is equivalent to solving erfc(-t / sqrt(2)) = 2 p.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         *  @warning If \p tolerance is set too low the procedure might never terminate.
         */
        expectation_type quantile(probability_type p, expectation_type tolerance = default_quantile_tolerance<expectation_type>) const
        {
            if (!aftermath::is_finite(p) || p < 0 || p > 1) throw std::logic_error("Probability must be a finite number between 0 and 1.");
            
            std::size_t count_steps = 0;
            constexpr std::size_t max_steps = 1000;

            expectation_type pp = static_cast<expectation_type>(p + p);
            expectation_type dx = 1 + tolerance;
            expectation_type x = 0;
            while (dx > tolerance || dx < -tolerance)
            {
                dx = (std::erfc(x) - pp) * std::exp(x * x) * math_constants<expectation_type>::root_pi_div_two;
                x += dx;
                ++count_steps;
                if (count_steps >= max_steps) break;
            } // while(...)
            return -math_constants<expectation_type>::root_two * x;
        } // quantile(...)

        /** Checks if the two distributions are the same. */
        constexpr bool operator ==(const type& other) const noexcept { return true; }

        /** Checks if the two distributions are different. */
        constexpr bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)
    }; // struct standard_normal_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            return 1;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_STANDARD_NORMAL_DISTRIBUTION_HPP_INCLUDED
