
#ifndef ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED

#include "probability/distribution_traits.hpp"

#include "probability/bernoulli_distribution.hpp"
#include "probability/binomial_distribution.hpp"
#include "probability/exponential_distribution.hpp"
#include "probability/normal_distribution.hpp"
#include "probability/standard_exponential_distribution.hpp"
#include "probability/standard_normal_distribution.hpp"

#include "probability/empirical_measure.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_distribution_type>
    inline constexpr bool is_continuous_v = aftermath::probability::is_continuous_v<t_distribution_type>;
    
    template <typename t_distribution_type>
    inline constexpr bool is_discrete_v = aftermath::probability::is_discrete_v<t_distribution_type>;

    template <typename t_distribution_type>
    inline constexpr bool has_left_tail_v = aftermath::probability::has_left_tail_v<t_distribution_type>;
    
    template <typename t_distribution_type>
    inline constexpr bool has_right_tail_v = aftermath::probability::has_right_tail_v<t_distribution_type>;
    
    template <typename t_distribution_type>
    inline constexpr bool has_bounded_support_v = aftermath::probability::has_bounded_support_v<t_distribution_type>;

    template <typename t_probability_type = typename aftermath::probability::bernoulli_distribution<>::probability_type,
        typename t_expectation_type = typename aftermath::probability::bernoulli_distribution<t_probability_type>::expectation_type>
    using bernoulli_distribution_t = aftermath::probability::bernoulli_distribution<t_probability_type, t_expectation_type>;

    template <typename t_value_type = typename aftermath::probability::binomial_distribution<>::value_type,
        typename t_probability_type = typename aftermath::probability::binomial_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename aftermath::probability::binomial_distribution<t_value_type, t_probability_type>::expectation_type>
    using binomial_distribution_t = aftermath::probability::binomial_distribution<t_value_type, t_probability_type, t_expectation_type>;
    
    template <typename t_value_type = typename aftermath::probability::exponential_distribution<>::value_type,
        typename t_probability_type = typename aftermath::probability::exponential_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename aftermath::probability::exponential_distribution<t_value_type, t_probability_type>::expectation_type>
    using exponential_distribution_t = aftermath::probability::exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
    
    template <typename t_value_type = typename aftermath::probability::normal_distribution<>::value_type,
        typename t_probability_type = typename aftermath::probability::normal_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename aftermath::probability::normal_distribution<t_value_type, t_probability_type>::expectation_type>
    using normal_distribution_t = aftermath::probability::normal_distribution<t_value_type, t_probability_type, t_expectation_type>;

    template <typename t_value_type = typename aftermath::probability::standard_exponential_distribution<>::value_type,
        typename t_probability_type = typename aftermath::probability::standard_exponential_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename aftermath::probability::standard_exponential_distribution<t_value_type, t_probability_type>::expectation_type>
    using standard_exponential_distribution_t = aftermath::probability::standard_exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
    
    template <typename t_value_type = typename aftermath::probability::standard_normal_distribution<>::value_type,
        typename t_probability_type = typename aftermath::probability::standard_normal_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename aftermath::probability::standard_normal_distribution<t_value_type, t_probability_type>::expectation_type>
    using standard_normal_distribution_t = aftermath::probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>;

    /** @breif A structure to record observations and build up statistics. */
    template <typename t_key_type,
        typename t_count_type = typename aftermath::probability::empirical_measure<t_key_type>::count_type,
        typename t_probability_type = typename aftermath::probability::empirical_measure<t_key_type, t_count_type>::probability_type,
        typename t_sum_type = typename aftermath::probability::empirical_measure<t_key_type, t_count_type, t_probability_type>::sum_type,
        typename t_mean_type = typename aftermath::probability::empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type>::mean_type>
    using empirical_measure_t = aftermath::probability::empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED
