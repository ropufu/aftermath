
#ifndef ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED

#include "random/sampler_bernoulli.hpp"
#include "random/sampler_binomial_alias.hpp"
#include "random/sampler_binomial_lookup.hpp"
#include "random/sampler_pareto.hpp"
#include "random/sampler_negative_pareto.hpp"
#include "random/sampler_normal.hpp"
#include "random/sampler_lognormal.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    using sampler_bernoulli_t = ropufu::aftermath::random::sampler_bernoulli<t_engine_type, t_result_type, t_param_type>;

    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    using sampler_binomial_alias_t = ropufu::aftermath::random::sampler_binomial_alias<t_engine_type, t_result_type, t_param_type>;

    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    using sampler_binomial_lookup_t = ropufu::aftermath::random::sampler_binomial_lookup<t_engine_type, t_result_type, t_param_type>;

    template <typename t_engine_type, typename t_result_type = double>
    using sampler_pareto_t = ropufu::aftermath::random::sampler_pareto<t_engine_type, t_result_type>;

    template <typename t_engine_type, typename t_result_type = double>
    using sampler_negative_pareto_t = ropufu::aftermath::random::sampler_negative_pareto<t_engine_type, t_result_type>;

    template <typename t_engine_type, typename t_result_type = double, std::size_t t_n_boxes = 1024>
    using sampler_normal_t = ropufu::aftermath::random::sampler_normal<t_engine_type, t_result_type, t_n_boxes>;

    template <typename t_engine_type, typename t_result_type = double, std::size_t t_n_boxes = 1024>
    using sampler_lognormal_t = ropufu::aftermath::random::sampler_lognormal<t_engine_type, t_result_type, t_n_boxes>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED
