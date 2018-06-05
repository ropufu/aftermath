
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED

#include "../probability/pareto_distribution.hpp"

#include <cmath>   // std::pow
#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double>
    struct sampler_pareto
    {
        using type = sampler_pareto<t_engine_type, t_result_type, t_probability_type>;

        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;
        
        using distribution_type = probability::pareto_distribution<t_result_type, t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        result_type m_x_min = 0;
        expectation_type m_alpha = 0;

    public:
        sampler_pareto() noexcept : sampler_pareto(distribution_type{}) { }

        explicit sampler_pareto(const distribution_type& distribution) noexcept
            : m_x_min(distribution.x_min()), m_alpha(distribution.alpha())
        {
        } // sampler_pareto(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            expectation_type uniform_random = static_cast<expectation_type>(uniform_generator() - engine_type::min()) / (static_cast<expectation_type>(type::diameter) + 1);
            return this->m_x_min / static_cast<result_type>(std::pow(1 - uniform_random, 1 / this->m_alpha));
        }
    }; // struct sampler_pareto
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED
