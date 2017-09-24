
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_NEGATIVE_PARETO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_NEGATIVE_PARETO_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../probability/dist_negative_pareto.hpp"
#include "../template_math.hpp"

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
            template <typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter>
            struct sampler_negative_pareto
            {
                static const t_bounds_type diameter = t_diameter;

                typedef probability::dist_negative_pareto distribution_type;
                typedef distribution_type::result_type    result_type;
                typedef t_uniform_type                    uniform_type;
                typedef t_bounds_type                     bounds_type;
                typedef sampler_negative_pareto<uniform_type, bounds_type, diameter> type;

                sampler_negative_pareto()
                    : sampler_negative_pareto(distribution_type())
                {
                }

                explicit sampler_negative_pareto(const distribution_type& distribution) = delete;

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator) = delete;
            };

            template <typename t_engine_type>
            using default_sampler_negative_pareto_t = sampler_negative_pareto<typename t_engine_type::result_type, std::size_t, t_engine_type::max() - t_engine_type::min()>;

            template <typename t_uniform_type>
            struct sampler_negative_pareto<t_uniform_type, std::size_t, mersenne_number<32>::value>
            {
                static const std::size_t diameter = mersenne_number<32>::value;

                typedef probability::dist_negative_pareto distribution_type;
                typedef distribution_type::result_type    result_type;
                typedef t_uniform_type                    uniform_type;
                typedef std::size_t                       bounds_type;
                typedef sampler_negative_pareto<uniform_type, bounds_type, diameter> type;

            private:
                double m_x_max, m_alpha;

            public:
                sampler_negative_pareto()
                    : sampler_negative_pareto(distribution_type())
                {
                }

                type& operator =(const type& other)
                {
                    if (this != &other)
                    {
                        this->m_x_max = other.m_x_max;
                        this->m_alpha = other.m_alpha;
                    }
                    return *this;
                }

                explicit sampler_negative_pareto(const distribution_type& distribution)
                    : m_x_max(distribution.x_max()), m_alpha(distribution.alpha())
                {

                }

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator)
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    double uniform_random = (uniform_generator() - t_engine_type::min()) / math_constants::two_pow_32;
                    return this->m_x_max * std::pow(1.0 - uniform_random, 1.0 / this->m_alpha);
                }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_NEGATIVE_PARETO_HPP_INCLUDED
