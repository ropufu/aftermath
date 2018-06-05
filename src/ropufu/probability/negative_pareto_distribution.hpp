
#ifndef ROPUFU_AFTERMATH_PROBABILITY_NEGATIVE_PARETO_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_NEGATIVE_PARETO_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval

namespace ropufu::aftermath::probability
{
    /** Negative Pareto distribution: a special case of scaled (0; x_max) Beta distribution with parameter beta = 1. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct negative_pareto_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Negative Pareto distribution: special case of scaled (0; x_max) Beta distribution with parameter beta = 1. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct negative_pareto_distribution
    {
        using type = negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = void;

        static constexpr char name[] = "negative_pareto"; // "negative pareto"

    private:
        expectation_type m_alpha = 1;
        value_type m_x_max = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_expected_value = static_cast<expectation_type>(0.5);
        expectation_type m_cache_variance = math_constants<expectation_type>::one_over_twelwe;
        expectation_type m_cache_standard_deviation = math_constants<expectation_type>::one_over_root_twelwe;
        expectation_type m_cache_axa = 1;

        bool validate(std::error_code& ec) const noexcept
        {
            if (std::isnan(this->m_alpha) || std::isinf(this->m_alpha) || this->m_alpha <= 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Shape must be positive.", false);
            if (std::isnan(this->m_x_max) || std::isinf(this->m_x_max) || this->m_x_max <= 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Scale must be positive.", false);
            return true;
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_expected_value = static_cast<expectation_type>(this->m_x_max * this->m_alpha / (this->m_alpha + 1));
            this->m_cache_variance = static_cast<expectation_type>(this->m_x_max * this->m_x_max / ((this->m_alpha + 1) * (this->m_alpha + 1) * (1 + 2 / this->m_alpha)));
            this->m_cache_standard_deviation = static_cast<expectation_type>(this->m_x_max / ((this->m_alpha + 1) * std::sqrt(1 + 2 / this->m_alpha)));
            this->m_cache_axa = static_cast<expectation_type>(this->m_alpha / std::pow(this->m_x_max, this->m_alpha));
        } // coerce(...)

    public:
        /** Default constructor with unit alpha and unit maximum. */
        negative_pareto_distribution() noexcept { }

        /** @brief Constructs a negative Pareto distribution from \p alpha and \p x_max.
         *  @param ec Set to std::errc::invalid_argument if \p alpha is not positive.
         *  @param ec Set to std::errc::invalid_argument if \p x_max is not positive.
         */
        explicit negative_pareto_distribution(expectation_type alpha, value_type x_max, std::error_code& ec) noexcept
            : m_alpha(alpha), m_x_max(x_max)
        {
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_alpha = 1;
                this->m_x_max = 1;
            } // if (...)
        } // negative_pareto_distribution(...)

        /** Shape parameter of the distribution. */
        expectation_type alpha() const noexcept { return this->m_alpha; }
        /** Scale parameter of the distribution. */
        value_type x_max() const noexcept { return this->m_x_max; }

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
        probability_type cdf(value_type x) const noexcept { return x <= 0 ? 0 : (x >= this->m_x_max ? 1 : static_cast<probability_type>(std::pow(x / this->m_x_max, this->m_alpha))); }
        
        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept { return (x <= 0 || x >= this->m_x_max) ? 0 : static_cast<expectation_type>(this->m_cache_axa * std::pow(x, this->m_alpha - 1)); }

        /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
        template <std::size_t t_nth_moment>
        expectation_type partial_moment(value_type a, value_type b) const noexcept
        {
            if (b <= 0 || a >= this->m_x_max) return 0;
            if (b > this->m_x_max) b = this->m_x_max;
            if (a < 0) a = 0;
        
            // this->m_cache_axa = this->m_xalpha / std::pow(this->m_x_max, this->m_alpha);
            return this->m_cache_axa * static_cast<expectation_type>(std::pow(b, t_nth_moment + this->m_alpha) - std::pow(a, t_nth_moment + this->m_alpha)) / static_cast<expectation_type>(t_nth_moment + this->m_alpha);
        } // partial_moment(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_alpha == other.m_alpha &&
                this->m_x_max == other.m_x_max;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)
    }; // struct negative_pareto_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type> 
    constexpr char negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::negative_pareto_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::value_type> value_hash = {};
            std::hash<typename argument_type::expectation_type> expectation_hash = {};

            return
                expectation_hash(x.alpha()) ^
                value_hash(x.x_max());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_NEGATIVE_PARETO_DISTRIBUTION_HPP_INCLUDED
