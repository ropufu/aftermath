
#ifndef ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED

#include "probability/distribution_traits.hpp"

#include "probability/binomial_distribution.hpp"
#include "probability/exponential_distribution.hpp"
#include "probability/lognormal_distribution.hpp"
#include "probability/negative_pareto_distribution.hpp"
#include "probability/normal_distribution.hpp"
#include "probability/pareto_distribution.hpp"
#include "probability/uniform_real_distribution.hpp"

#include "probability/empirical_measure.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_distribution_type>
    inline constexpr bool is_continuous_v = ropufu::aftermath::probability::is_continuous_v<t_distribution_type>;
    
    template <typename t_distribution_type>
    inline constexpr bool is_discrete_v = ropufu::aftermath::probability::is_discrete_v<t_distribution_type>;

    template <typename t_size_type = std::size_t, typename t_probability_type = double>
    using binomial_distribution_t = ropufu::aftermath::probability::binomial_distribution<t_size_type, t_probability_type>;
    
    template <typename t_result_type = double>
    using exponential_distribution_t = ropufu::aftermath::probability::exponential_distribution<t_result_type>;
    
    template <typename t_result_type = double>
    using lognormal_distribution_t = ropufu::aftermath::probability::lognormal_distribution<t_result_type>;
    
    template <typename t_result_type = double>
    using negative_pareto_distribution_t = ropufu::aftermath::probability::negative_pareto_distribution<t_result_type>;
    
    template <typename t_result_type = double>
    using normal_distribution_t = ropufu::aftermath::probability::normal_distribution<t_result_type>;
    
    template <typename t_result_type = double>
    using pareto_distribution_t = ropufu::aftermath::probability::pareto_distribution<t_result_type>;
    
    template <typename t_result_type = double>
    using uniform_real_distribution_t = ropufu::aftermath::probability::uniform_real_distribution<t_result_type>;
    
    /** @breif A structure to record observations and build statistics.
     *  @remark The general struct implements the basic functionality meant to be shared amond specializations.
     */
    template <typename t_key_type,
        typename t_count_type = std::size_t,
        typename t_probability_type = double,
        typename t_sum_type = ropufu::aftermath::probability::detail::product_result_t<t_key_type, t_count_type>,
        typename t_mean_type = ropufu::aftermath::probability::detail::product_result_t<t_key_type, t_probability_type>>
    using empirical_measure_t = ropufu::aftermath::probability::empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_PROBABILITY_HPP_INCLUDED
