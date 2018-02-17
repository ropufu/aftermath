
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../not_an_error.hpp"
#include "traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow, std::erfc
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::normal_distribution

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Normal (Gaussian) distribution. */
            template <typename t_result_type = double>
            struct dist_normal;

            template <typename t_result_type>
            using dist_gaussian = dist_normal<t_result_type>;

            template <typename t_result_type>
            struct is_continuous<dist_normal<t_result_type>>
            {
                using distribution_type = dist_normal<t_result_type>;
                static constexpr bool value = true;
            }; // struct is_continuous

            /** @brief Normal (Gaussian) distribution.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            template <typename t_result_type>
            struct dist_normal
            {
                using type = dist_normal<t_result_type>;
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

                void coerce() noexcept
                {
                    if (std::isnan(this->m_mu) || std::isinf(this->m_mu))
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Mu must be finite. Coerced to 0.", __FUNCTION__, __LINE__);
                        this->m_mu = 0;
                    }
                    if (std::isnan(this->m_sigma) || std::isinf(this->m_sigma) || this->m_sigma <= 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Sigma must be positive. Coerced to 1.", __FUNCTION__, __LINE__);
                        this->m_sigma = 1;
                    }
                    this->m_cache_variance = this->m_sigma * this->m_sigma;
                    this->m_cache_sigma_root_two = this->m_sigma * math_constants<result_type>::root_two; // sigma * std::sqrt(2)
                    this->m_cache_pdf_scale = math_constants<result_type>::one_div_root_two_pi / this->m_sigma; // 1 / (sigma * std::sqrt(2 pi))
                } // coerce(...)

            public:
                /** Default constructor with zero mean and unit variance. */
                dist_normal() noexcept { }

                /** Constructor and implicit conversion from standard distribution. */
                dist_normal(const std_type& distribution) noexcept : dist_normal(static_cast<param_type>(distribution.mean()), static_cast<param_type>(distribution.stddev())) { }

                /** @brief Constructs a normal distribution from the mean and standard deviation.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p sigma is not positive.
                 */
                explicit dist_normal(param_type mu, param_type sigma = 1) noexcept
                    : m_mu(mu), m_sigma(sigma)
                {
                    this->coerce();
                } // dist_normal

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
            }; // struct dist_normal

            // ~~ Definitions ~~
            template <typename t_result_type> constexpr char dist_normal<t_result_type>::name[];
            template <typename t_result_type>
            const dist_normal<t_result_type> dist_normal<t_result_type>::standard = dist_normal<t_result_type>();
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

namespace std
{
    template <typename t_result_type>
    struct hash<ropufu::aftermath::probability::dist_normal<t_result_type>>
    {
        using argument_type = ropufu::aftermath::probability::dist_normal<t_result_type>;
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

#endif // ROPUFU_AFTERMATH_PROBABILITY_DIST_NORMAL_HPP_INCLUDED
