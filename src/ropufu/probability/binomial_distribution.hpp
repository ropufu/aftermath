
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED

#include "distribution_traits.hpp"

#include <cmath>      // std::isnan, std::isinf, std::sqrt, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::binomial_distribution
#include <stdexcept>  // std::out_of_range

namespace ropufu::aftermath::probability
{
    /** Binomial distribution. */
    template <typename t_size_type = std::size_t, typename t_probability_type = double>
    struct binomial_distribution;

    template <typename t_size_type, typename t_probability_type>
    struct is_discrete<binomial_distribution<t_size_type, t_probability_type>>
    {
        using distribution_type = binomial_distribution<t_size_type, t_probability_type>;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Binomial distribution. */
    template <typename t_size_type, typename t_probability_type>
    struct binomial_distribution
    {
        using type = binomial_distribution<t_size_type, t_probability_type>;
        using size_type = t_size_type;
        using probability_type = t_probability_type;
        using result_type = t_size_type;
        using param_type = t_probability_type;
        using std_type = std::binomial_distribution<t_size_type>;

        static constexpr char name[] = "binomial";

    private:
        size_type m_number_of_trials = 1;
        probability_type m_probability_of_success = 0;
        // ~~ Cached values ~~
        param_type m_cache_probability_of_failure = 1;
        param_type m_cache_expected_value = 0;
        param_type m_cache_standard_deviation = 0;
        param_type m_cache_variance = 0;

        void coerce()
        {
            if (this->m_number_of_trials <= 0) throw std::out_of_range("Number of trials must be positive.");
            if (std::isnan(this->m_probability_of_success) || std::isinf(this->m_probability_of_success) ||
                this->m_probability_of_success < 0 || this->m_probability_of_success > 1) throw std::out_of_range("Probability of success must be in the range from 0 to 1.");
                
            this->m_cache_probability_of_failure = 1 - this->m_probability_of_success;
            this->m_cache_expected_value = this->m_number_of_trials * this->m_probability_of_success;
            this->m_cache_variance = this->m_cache_expected_value * this->m_cache_probability_of_failure;
            this->m_cache_standard_deviation = std::sqrt(m_cache_variance);
        } // coerce(...)

    public:
        /** Default constructor with one trial and probability of success 1/2. */
        binomial_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        binomial_distribution(const std_type& distribution) noexcept : binomial_distribution(distribution.t(), static_cast<param_type>(distribution.p())) { }

        /** @brief Constructs a binomial distribution from the number of trials, \p n, and probability of success.
         *  @exception std::out_of_range \p probability_of_success is not in the interval [0, 1].
         */
        explicit binomial_distribution(size_type number_of_trials, probability_type probability_of_success)
            : m_number_of_trials(number_of_trials), m_probability_of_success(probability_of_success)
        {
            this->coerce();
        } // binomial_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_number_of_trials, static_cast<typename std_type::param_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Number of trials in the experiment. */
        size_type number_of_trials() const noexcept { return this->m_number_of_trials; }
        /** Probability of success. */
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }
        /** Probability of failure. */
        probability_type probability_of_failure() const noexcept { return this->m_cache_probability_of_failure; }

        /** Expected value of the distribution. */
        param_type expected_value() const noexcept { return this->m_cache_expected_value; }
        /** Variance of the distribution. */
        param_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        param_type standard_deviation() const noexcept { return this->m_cache_standard_deviation; }

        /** Expected value of the distribution. */
        param_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        param_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(result_type k) const noexcept
        {
            if (k >= this->m_number_of_trials) return 1;
            
            param_type p = 0;
            for (result_type j = 0; j <= k; ++j) p += this->pdf(j);
            return p;
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pdf(result_type k) const noexcept
        {
            return type::n_choose_k(this->m_number_of_trials, k) * static_cast<param_type>(std::pow(this->m_probability_of_success, k) * std::pow(this->m_cache_probability_of_failure, this->m_number_of_trials - k));
        } // pdf(...)

        /** Binomial coefficient. */
        static param_type n_choose_k(result_type n, result_type k) noexcept
        {
            if (k > n) return 0;
            if (k > n / 2) k = n - k;

            param_type result = 1;
            result_type numerator = n - k;
            for (result_type i = 1; i <= k; ++i) result *= static_cast<param_type>(++numerator) / i;
            return result;
        } // n_choose_k(...)

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
    template <typename t_size_type, typename t_probability_type> constexpr char binomial_distribution<t_size_type, t_probability_type>::name[];
} // namespace ropufu::aftermath::probability

namespace std
{
    template <typename t_size_type, typename t_probability_type>
    struct hash<ropufu::aftermath::probability::binomial_distribution<t_size_type, t_probability_type>>
    {
        using argument_type = ropufu::aftermath::probability::binomial_distribution<t_size_type, t_probability_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            std::hash<typename argument_type::result_type> result_hash = { };
            std::hash<typename argument_type::param_type> param_hash = { };

            return
                result_hash(x.number_of_trials()) ^ 
                param_hash(x.probability_of_success());
        } // operator ()(...)
    }; // struct hash
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
