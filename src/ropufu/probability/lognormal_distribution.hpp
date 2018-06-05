
#ifndef ROPUFU_AFTERMATH_PROBABILITY_LOGNORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_LOGNORMAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow, std::erfc, std::log
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::lognormal_distribution
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval

namespace ropufu::aftermath::probability
{
    /** Lognormal distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct lognormal_distribution;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Lognormal distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct lognormal_distribution
    {
        using type = lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::lognormal_distribution<t_value_type>;

        static constexpr char name[] = "lognorm"; // "lognormal"

    private:
        expectation_type m_mu = 0;
        expectation_type m_sigma = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_expected_value = math_constants<expectation_type>::root_e;
        expectation_type m_cache_variance = (math_constants<expectation_type>::e - 1) * math_constants<expectation_type>::root_e;
        expectation_type m_cache_standard_deviation = (math_constants<expectation_type>::root_e_minus_one) * math_constants<expectation_type>::root_e;
        expectation_type m_cache_sigma_squared = 1;
        expectation_type m_cache_sigma_root_two = math_constants<expectation_type>::root_two;
        expectation_type m_cache_pdf_scale = math_constants<expectation_type>::one_div_root_two_pi;

        bool validate(std::error_code& ec) const noexcept
        {
            if (std::isnan(this->m_mu) || std::isinf(this->m_mu)) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Mu must be finite.", false);
            if (std::isnan(this->m_sigma) || std::isinf(this->m_sigma) || this->m_sigma <= 0) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Sigma must be positive.", false);
            return true;
        } // validate(...)

        void cahce() noexcept
        {
            expectation_type v = this->m_sigma * this->m_sigma;

            this->m_cache_expected_value = static_cast<expectation_type>(std::exp(this->m_mu + v / 2));
            this->m_cache_variance = static_cast<expectation_type>((std::exp(v) - 1) * std::exp(2 * this->m_mu + v));
            this->m_cache_standard_deviation = static_cast<expectation_type>(std::sqrt(std::exp(v) - 1) * std::exp(this->m_mu + v / 2));
            this->m_cache_sigma_squared = v;
            this->m_cache_sigma_root_two = this->m_sigma * math_constants<expectation_type>::root_two; // sigma * std::sqrt(2)
            this->m_cache_pdf_scale = math_constants<expectation_type>::one_div_root_two_pi / this->m_sigma; // 1 / (sigma * std::sqrt(2 pi))
        } // coerce(...)

    public:
        /** Default constructor with unit sigma. */
        lognormal_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ lognormal_distribution(const std_type& distribution) noexcept
            : m_mu(static_cast<expectation_type>(distribution.m())), m_sigma(static_cast<expectation_type>(distribution.s()))
        {
            this->cahce();
        } // lognormal_distribution(...)

        /** @brief Constructs a lognormal distribution from mu and sigma, the mean and standard deviation of the corresponding normal distribution.
         *  @param ec Set to std::errc::invalid_argument if \p sigma is not positive.
         */
        explicit lognormal_distribution(expectation_type mu, expectation_type sigma, std::error_code& ec) noexcept
            : m_mu(mu), m_sigma(sigma)
        {
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_mu = 0;
                this->m_sigma = 1;
            } // if (...)
        } // lognormal_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_mu, this->m_sigma);
        } // to_std(...)

        /** Mean of the underlying normal distribution. */
        expectation_type mu() const noexcept { return this->m_mu; }
        /** Standard deviation of the underlying normal distribution. */
        expectation_type sigma() const noexcept { return this->m_sigma; }

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
        probability_type cdf(value_type x) const noexcept { return static_cast<probability_type>(std::erfc((this->m_mu - std::log(x)) / (this->m_cache_sigma_root_two)) / 2); }
        
        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return static_cast<expectation_type>((this->m_cache_pdf_scale / x) * std::exp(-(std::log(x) - this->m_mu) * (std::log(x) - this->m_mu) / (2 * this->m_cache_variance)));
        } // pdf(...)

        /** Partial n-th moment of the distribution: expected value restricted to the interval [a, b]. */
        template <std::size_t t_nth_moment>
        expectation_type partial_moment(value_type a, value_type b) const noexcept
        {
            return static_cast<expectation_type>(std::exp(t_nth_moment * this->m_mu + t_nth_moment * t_nth_moment * this->m_cache_sigma_squared / 2) * (
                std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(a)) / (this->m_cache_sigma_root_two)) -
                std::erf((this->m_mu + t_nth_moment * this->m_cache_sigma_squared - std::log(b)) / (this->m_cache_sigma_root_two))) / 2);
        } // partial_moment(...)

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
    }; // struct lognormal_distribution

    // ~~ Definitions ~~
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type> 
    constexpr char lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using argument_type = ropufu::aftermath::probability::lognormal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::expectation_type> expectation_hash = {};

            return
                expectation_hash(x.mu()) ^
                expectation_hash(x.sigma());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_LOGNORMAL_DISTRIBUTION_HPP_INCLUDED
