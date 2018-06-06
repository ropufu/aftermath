
#ifndef ROPUFU_AFTERMATH_RANDOM_NEGATIVE_PARETO_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_NEGATIVE_PARETO_SAMPLER_HPP_INCLUDED

#include "../probability/negative_pareto_distribution.hpp"

#include <cmath>   // std::pow
#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double>
    struct negative_pareto_sampler
    {
        using type = negative_pareto_sampler<t_engine_type, t_result_type, t_probability_type>;

        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;

        using distribution_type = probability::negative_pareto_distribution<t_result_type, t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        static constexpr expectation_type norm = static_cast<expectation_type>(type::diameter) + 1;

    private:
        result_type m_x_max = 1;
        expectation_type m_alpha = 1;

    public:
        negative_pareto_sampler() noexcept { }

        explicit negative_pareto_sampler(const distribution_type& distribution) noexcept
            : m_x_max(distribution.x_max()), m_alpha(distribution.alpha())
        {
        } // negative_pareto_sampler(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            expectation_type uniform_random = static_cast<expectation_type>(uniform_generator() - engine_type::min()) / (type::norm);
            return this->m_x_max * static_cast<result_type>(std::pow(1 - uniform_random, 1 / this->m_alpha));
        } // operator ()(...)
    }; // struct negative_pareto_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_NEGATIVE_PARETO_SAMPLER_HPP_INCLUDED
