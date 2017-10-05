
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED

#include "../probability/dist_lognormal.hpp"
#include "../probability/dist_normal.hpp"
#include "ziggurat_normal.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            template <std::uint_fast32_t t_n_boxes, typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter>
            struct sampler_lognormal
            {
                static constexpr std::uint_fast32_t n_boxes = t_n_boxes;
                static constexpr t_bounds_type diameter = t_diameter;

                using type = sampler_lognormal<t_n_boxes, t_uniform_type, t_bounds_type, diameter>;
                using distribution_type = probability::dist_lognormal;
                using result_type = distribution_type::result_type;
                using uniform_type = t_uniform_type;
                using bounds_type = t_bounds_type;
                using ziggurat_type = ziggurat_normal<n_boxes, uniform_type, bounds_type, diameter>;

            private:
                double m_mu, m_sigma;
                ziggurat_type m_ziggurat;

            public:
                sampler_lognormal() noexcept
                    : sampler_lognormal(distribution_type())
                {
                }

                type& operator =(const type& other) noexcept
                {
                    if (this != &other)
                    {
                        this->m_mu = other.m_mu;
                        this->m_sigma = other.m_sigma;
                        //this->m_ziggurat = other.m_ziggurat;
                    }
                    return *this;
                }

                explicit sampler_lognormal(const distribution_type& distribution) noexcept
                    : m_mu(distribution.mu()), m_sigma(distribution.sigma()), m_ziggurat()
                {

                }

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator) noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    result_type normal_random = this->m_ziggurat.sample(uniform_generator);
                    return std::exp(this->m_mu + this->m_sigma * normal_random);
                }
            };

            template <typename t_engine_type>
            using default_sampler_lognormal_t = sampler_lognormal<
                default_ziggurat_normal_t<t_engine_type>::n_boxes, 
                typename default_ziggurat_normal_t<t_engine_type>::uniform_type,
                typename default_ziggurat_normal_t<t_engine_type>::bounds_type,
                default_ziggurat_normal_t<t_engine_type>::diameter>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_LOGNORMAL_HPP_INCLUDED
