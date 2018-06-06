
#ifndef ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../on_error.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow, std::erfc
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::normal_distribution
#include <system_error> // std::error_code, std::errc
#include <utility>    // std::declval

namespace ropufu::aftermath::probability
{
    /** Normal (Gaussian) distribution. */
    template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct normal_distribution;

    // template <typename t_value_type = double, typename t_probability_type = t_value_type, typename t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    // using gaussian_distribution_t = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct is_continuous<normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    {
        using distribution_type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Normal (Gaussian) distribution. */
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct normal_distribution
    {
        using type = normal_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::normal_distribution<t_value_type>;

        static constexpr char name[] = "norm"; // "normal"

        static const type standard;

    private:
        expectation_type m_mu = 0;
        expectation_type m_sigma = 1;
        // ~~ Cached values ~~
        expectation_type m_cache_variance = 1;
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
            this->m_cache_variance = this->m_sigma * this->m_sigma;
            this->m_cache_sigma_root_two = this->m_sigma * math_constants<expectation_type>::root_two; // sigma * std::sqrt(2)
            this->m_cache_pdf_scale = math_constants<expectation_type>::one_div_root_two_pi / this->m_sigma; // 1 / (sigma * std::sqrt(2 pi))
        } // coerce(...)

    public:
        /** Default constructor with zero mean and unit variance. */
        normal_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ normal_distribution(const std_type& distribution) noexcept
            : m_mu(static_cast<expectation_type>(distribution.mean())), m_sigma(static_cast<expectation_type>(distribution.stddev()))
        {
            this->cahce();
        } // normal_distribution(...)

        /** @brief Constructs a normal distribution from the mean and standard deviation.
         *  @param ec Set to std::errc::invalid_argument if \p sigma is not positive.
         */
        explicit normal_distribution(expectation_type mu, expectation_type sigma, std::error_code& ec) noexcept
            : m_mu(mu), m_sigma(sigma)
        {
            if (this->validate(ec)) this->cahce();
            else
            {
                this->m_mu = 0;
                this->m_sigma = 1;
            } // if (...)
        } // normal_distribution

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_mu, this->m_sigma);
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
        probability_type cdf(value_type x) const noexcept { return static_cast<probability_type>(std::erfc((this->m_mu - x) / (this->m_cache_sigma_root_two)) / 2); }

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return this->m_cache_pdf_scale * static_cast<expectation_type>(std::exp(-(x - this->m_mu) * (x - this->m_mu) / (2 * this->m_cache_variance)));
        } // pdf(...)

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

    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    const normal_distribution<t_value_type, t_probability_type, t_expectation_type> normal_distribution<t_value_type, t_probability_type, t_expectation_type>::standard = {};
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
                expectation_hash(x.mu()) ^
                expectation_hash(x.sigma());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
