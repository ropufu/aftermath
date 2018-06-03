
#ifndef ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED

#include "../math_constants.hpp"
#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow, std::erfc
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::normal_distribution
#include <stdexcept>  // std::out_of_range

namespace ropufu::aftermath::probability
{
    /** Normal (Gaussian) distribution. */
    template <typename t_result_type = double>
    struct normal_distribution;

    template <typename t_result_type>
    using dist_gaussian = normal_distribution<t_result_type>;

    template <typename t_result_type>
    struct is_continuous<normal_distribution<t_result_type>>
    {
        using distribution_type = normal_distribution<t_result_type>;
        static constexpr bool value = true;
    }; // struct is_continuous

    /** @brief Normal (Gaussian) distribution. */
    template <typename t_result_type>
    struct normal_distribution
    {
        using type = normal_distribution<t_result_type>;
        using result_type = t_result_type;
        using param_type = t_result_type;
        using std_type = std::normal_distribution<t_result_type>;

        static constexpr char name[] = "norm"; // "normal"

        static const type standard;

    private:
        param_type m_mu = 0;
        param_type m_sigma = 1;
        // ~~ Cached values ~~
        param_type m_cache_variance = 1;
        param_type m_cache_sigma_root_two = math_constants<result_type>::root_two;
        param_type m_cache_pdf_scale = math_constants<result_type>::one_div_root_two_pi;

        void coerce()
        {
            if (std::isnan(this->m_mu) || std::isinf(this->m_mu)) throw std::out_of_range("Mu must be finite.");
            if (std::isnan(this->m_sigma) || std::isinf(this->m_sigma) || this->m_sigma <= 0) throw std::out_of_range("Sigma must be positive.");
            
            this->m_cache_variance = this->m_sigma * this->m_sigma;
            this->m_cache_sigma_root_two = this->m_sigma * math_constants<result_type>::root_two; // sigma * std::sqrt(2)
            this->m_cache_pdf_scale = math_constants<result_type>::one_div_root_two_pi / this->m_sigma; // 1 / (sigma * std::sqrt(2 pi))
        } // coerce(...)

    public:
        /** Default constructor with zero mean and unit variance. */
        normal_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        normal_distribution(const std_type& distribution) noexcept : normal_distribution(static_cast<param_type>(distribution.mean()), static_cast<param_type>(distribution.stddev())) { }

        /** @brief Constructs a normal distribution from the mean and standard deviation.
         *  @exception std::out_of_range \p sigma is not positive.
         */
        explicit normal_distribution(param_type mu, param_type sigma = 1)
            : m_mu(mu), m_sigma(sigma)
        {
            this->coerce();
        } // normal_distribution

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_mu, this->m_sigma);
        } // to_std(...)

        /** Mean of the distribution. */
        param_type mu() const noexcept { return this->m_mu; }
        /** Standard deviation of the distribution. */
        param_type sigma() const noexcept { return this->m_sigma; }

        /** Expected value of the distribution. */
        param_type expected_value() const noexcept { return this->m_mu; }
        /** Variance of the distribution. */
        param_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        param_type standard_deviation() const noexcept { return this->m_sigma; }

        /** Expected value of the distribution. */
        param_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        param_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */   
        param_type cdf(result_type x) const noexcept { return std::erfc((this->m_mu - x) / (this->m_cache_sigma_root_two)) / 2; }

        /** Probability density function (p.d.f.) of the distribution. */
        param_type pdf(result_type x) const noexcept
        {
            return this->m_cache_pdf_scale * std::exp(-(x - this->m_mu) * (x - this->m_mu) / (2 * this->m_cache_variance));
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
    template <typename t_result_type> constexpr char normal_distribution<t_result_type>::name[];
    template <typename t_result_type>
    const normal_distribution<t_result_type> normal_distribution<t_result_type>::standard = normal_distribution<t_result_type>();
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::normal_distribution<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::normal_distribution<t_result_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                param_hash(x.mu()) ^
                param_hash(x.sigma());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_NORMAL_DISTRIBUTION_HPP_INCLUDED
