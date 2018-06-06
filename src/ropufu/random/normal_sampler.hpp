
#ifndef ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_HPP_INCLUDED

#include "../probability/normal_distribution.hpp"
#include "ziggurat_normal.hpp"

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = double, typename t_probability_type = double, std::size_t t_n_boxes = 1024>
    struct normal_sampler
    {
        using type = normal_sampler<t_engine_type, t_result_type, t_probability_type, t_n_boxes>;
        using ziggurat_type = ziggurat_normal<t_engine_type, t_result_type, t_probability_type, t_n_boxes>;

        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;
        
        using distribution_type = probability::normal_distribution<t_result_type, t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr std::size_t n_boxes = t_n_boxes;
        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        static constexpr expectation_type norm = static_cast<expectation_type>(type::diameter) + 1;

    private:
        expectation_type m_mu = 0;
        expectation_type m_sigma = 0;
        ziggurat_type m_ziggurat = {};

    public:
        normal_sampler() noexcept { }

        explicit normal_sampler(const distribution_type& distribution) noexcept
            : m_mu(distribution.mu()), m_sigma(distribution.sigma())
        {
        } // normal_sampler(...)

        result_type operator ()(engine_type& uniform_generator) noexcept
        {
            result_type z = this->m_ziggurat.sample(uniform_generator);
            return static_cast<result_type>(this->m_mu + this->m_sigma * z);
        } // operator ()(...)
    }; // struct normal_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_HPP_INCLUDED
