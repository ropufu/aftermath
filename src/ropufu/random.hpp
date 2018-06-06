
#ifndef ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED

#include "random/bernoulli_sampler.hpp"
#include "random/binomial_alias_sampler.hpp"
#include "random/binomial_lookup.hpp"
#include "random/pareto_sampler.hpp"
#include "random/negative_pareto_sampler.hpp"
#include "random/normal_sampler.hpp"
#include "random/lognormal_sampler.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_probability_type = double>
    using bernoulli_sampler_t = ropufu::aftermath::random::bernoulli_sampler<t_engine_type, t_result_type, t_probability_type>;

    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_probability_type = double>
    using binomial_alias_sampler_t = ropufu::aftermath::random::binomial_alias_sampler<t_engine_type, t_result_type, t_probability_type>;

    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_probability_type = double>
    using binomial_lookup_t = ropufu::aftermath::random::binomial_lookup<t_engine_type, t_result_type, t_probability_type>;

    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double>
    using pareto_sampler_t = ropufu::aftermath::random::pareto_sampler<t_engine_type, t_result_type, t_probability_type>;

    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double>
    using negative_pareto_sampler_t = ropufu::aftermath::random::negative_pareto_sampler<t_engine_type, t_result_type, t_probability_type>;

    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double, std::size_t t_n_boxes = 1024>
    using normal_sampler_t = ropufu::aftermath::random::normal_sampler<t_engine_type, t_result_type, t_probability_type, t_n_boxes>;

    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double, std::size_t t_n_boxes = 1024>
    using gaussian_sampler_t = ropufu::aftermath::random::normal_sampler<t_engine_type, t_result_type, t_probability_type, t_n_boxes>;

    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double, std::size_t t_n_boxes = 1024>
    using lognormal_sampler_t = ropufu::aftermath::random::lognormal_sampler<t_engine_type, t_result_type, t_probability_type, t_n_boxes>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_RANDOM_HPP_INCLUDED
