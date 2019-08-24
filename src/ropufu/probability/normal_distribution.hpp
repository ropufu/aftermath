
#ifndef ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../number_traits.hpp"
#include "distribution_traits.hpp"
#include "standard_normal_distribution.hpp"

#include <cmath>       // std::sqrt, std::pow, std::erfc
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <random>      // std::normal_distribution
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval

namespace ropufu::aftermath::probability
{
    /** Normal (Gaussian) distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct normal_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct has_right_tail<normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct has_right_tail

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct has_left_tail<normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct has_left_tail

    /** @brief Normal (Gaussian) distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct normal_distribution
    {
        using type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using fundamental_type = standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::normal_distribution<t_value_type>;

        static constexpr char name[] = "norm"; // "normal"

    private:
        fundamental_type m_fundamental_distribution = {};
        expectation_type m_mu = 0;
        expectation_type m_sigma = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_variance = 1;
        expectation_type m_cache_pdf_scale = 1;
        value_type m_cache_one_div_sigma = 1;
        value_type m_cache_mu_div_sigma = 0;

        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_floating_point_v<value_type>, "Value type has to be a floating point type.");
            static_assert(std::is_floating_point_v<probability_type>, "Probability type has to be a floating point type.");
            static_assert(std::is_floating_point_v<expectation_type>, "Expectation type has to be a floating point type.");
        } // traits_check(...)

        void validate() const
        {
            if (!aftermath::is_finite(this->m_mu)) throw std::logic_error("Mu must be finite.");
            if (!aftermath::is_finite(this->m_sigma) || this->m_sigma <= 0) throw std::logic_error("Sigma must be positive.");
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_variance = this->m_sigma * this->m_sigma;
            this->m_cache_pdf_scale = 1 / this->m_sigma;
            this->m_cache_one_div_sigma = static_cast<value_type>(1 / this->m_sigma);
            this->m_cache_mu_div_sigma = static_cast<value_type>(this->m_mu / this->m_sigma);
        } // coerce(...)

    public:
        /** Default constructor with zero mean and unit variance. */
        normal_distribution() noexcept { type::traits_check(); }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ normal_distribution(const std_type& distribution)
            : normal_distribution(static_cast<expectation_type>(distribution.mean()), static_cast<expectation_type>(distribution.stddev()))
        {
        } // normal_distribution(...)

        /** @brief Constructs a normal distribution from the mean and standard deviation.
         *  @exception std::logic_error \p sigma is not positive.
         */
        normal_distribution(expectation_type mu, expectation_type sigma)
            : m_mu(mu), m_sigma(sigma)
        {
            type::traits_check();

            this->validate();
            this->cahce();
        } // normal_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using mu_type = decltype(std::declval<std_type>().mean());
            using sigma_type = decltype(std::declval<std_type>().sigma());
            return std_type(static_cast<mu_type>(this->m_mu), static_cast<sigma_type>(this->m_sigma));
        } // to_std(...)

        /** Mean of the distribution. */
        expectation_type mu() const noexcept { return this->m_mu; }
        /** Standard deviation of the distribution. */
        expectation_type sigma() const noexcept { return this->m_sigma; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_mu; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_sigma; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */   
        probability_type cdf(value_type x) const noexcept
        {
            return this->m_fundamental_distribution.cdf(this->m_cache_one_div_sigma * x - this->m_cache_mu_div_sigma);
        } // cdf(...)

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return this->m_cache_pdf_scale * this->m_fundamental_distribution.pdf(this->m_cache_one_div_sigma * x - this->m_cache_mu_div_sigma);
        } // pdf(...)
        
        /** @brief For a given 0 <= p <= 1, find t such that cdf(t) = p.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         *  @warning If \p tolerance is set too low the procedure might never terminate.
         */
        expectation_type quantile(probability_type p, expectation_type tolerance = default_quantile_tolerance<expectation_type>) const
        {
            return this->m_sigma * this->m_fundamental_distribution.quantile(p, tolerance) + this->m_mu;
        } // quantile(...)

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
    }; // struct normal_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    constexpr char normal_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::expectation_type> expectation_hash = {};
            return
                (expectation_hash(x.mu()) << 4) ^
                (expectation_hash(x.sigma()));
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
