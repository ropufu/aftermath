
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED

#include "../probability/lognormal_distribution.hpp"
#include "ziggurat_normal.hpp"

#include <cmath>   // std::exp
#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = double, std::size_t t_n_boxes = 1024>
    struct sampler_lognormal
    {
        using type = sampler_lognormal<t_engine_type, t_result_type, t_n_boxes>;
        using ziggurat_type = ziggurat_normal<t_engine_type, t_result_type, t_n_boxes>;

        using engine_type = t_engine_type;
        using distribution_type = probability::lognormal_distribution<t_result_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr std::size_t n_boxes = t_n_boxes;
        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        param_type m_mu = 0;
        param_type m_sigma = 0;
        ziggurat_type m_ziggurat = {};

    public:
        sampler_lognormal() noexcept : sampler_lognormal({}) { }

        explicit sampler_lognormal(const distribution_type& distribution) noexcept
            : m_mu(distribution.mu()), m_sigma(distribution.sigma())
        {
        } // sampler_lognormal(...)

        result_type operator ()(engine_type& uniform_generator) noexcept
        {
            result_type z = this->m_ziggurat.sample(uniform_generator);
            return std::exp(this->m_mu + this->m_sigma * z);
        } // operator ()(...)
    }; // struct sampler_lognormal
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED
