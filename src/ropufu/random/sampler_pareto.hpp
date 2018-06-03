
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED

#include "../probability/pareto_distribution.hpp"

#include <cmath>   // std::pow
#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = double>
    struct sampler_pareto
    {
        using type = sampler_pareto<t_engine_type, t_result_type>;
        using engine_type = t_engine_type;
        using distribution_type = probability::pareto_distribution<t_result_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        param_type m_x_min = 0;
        param_type m_alpha = 0;

    public:
        sampler_pareto() noexcept : sampler_pareto({}) { }

        explicit sampler_pareto(const distribution_type& distribution) noexcept
            : m_x_min(distribution.x_min()), m_alpha(distribution.alpha())
        {
        } // sampler_pareto(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            param_type uniform_random = (uniform_generator() - engine_type::min()) / (static_cast<param_type>(type::diameter) + 1);
            return this->m_x_min / std::pow(1 - uniform_random, 1 / this->m_alpha);
        }
    }; // struct sampler_pareto
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_PARETO_HPP_INCLUDED
