
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_NORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_NORMAL_HPP_INCLUDED

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
            struct sampler_normal
            {
                typedef sampler_normal<t_n_boxes, t_uniform_type, t_bounds_type, t_diameter> type;

                static constexpr std::uint_fast32_t n_boxes = t_n_boxes;
                static constexpr t_bounds_type diameter = t_diameter;

                typedef probability::dist_normal       distribution_type;
                typedef distribution_type::result_type result_type;
                typedef t_uniform_type                 uniform_type;
                typedef t_bounds_type                  bounds_type;
                typedef ziggurat_normal<n_boxes, uniform_type, bounds_type, diameter> ziggurat_type;

            private:
                double m_mu, m_sigma;
                ziggurat_type m_ziggurat;

            public:
                sampler_normal() noexcept
                    : sampler_normal(distribution_type())
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

                explicit sampler_normal(const distribution_type& distribution) noexcept
                    : m_mu(distribution.mu()), m_sigma(distribution.sigma()), m_ziggurat()
                {

                }

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator) noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    result_type standard_random = this->m_ziggurat.sample(uniform_generator);
                    return this->m_mu + this->m_sigma * standard_random;
                }
            };

            template <typename t_engine_type>
            using default_sampler_normal_t = sampler_normal<
                default_ziggurat_normal_t<t_engine_type>::n_boxes, 
                typename default_ziggurat_normal_t<t_engine_type>::uniform_type,
                typename default_ziggurat_normal_t<t_engine_type>::bounds_type,
                default_ziggurat_normal_t<t_engine_type>::diameter>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_NORMAL_HPP_INCLUDED
