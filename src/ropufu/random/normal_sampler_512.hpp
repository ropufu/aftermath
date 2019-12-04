
#ifndef ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED

#include "../probability/standard_normal_distribution.hpp"
#include "ziggurat_layers_normal_512_m24.hpp"
#include "ziggurat_layers_normal_512_2147483645.hpp"
#include "ziggurat_layers_normal_512_m32.hpp"
#include "ziggurat_layers_normal_512_m48.hpp"
#include "ziggurat_layers_normal_512_m64.hpp"
#include "ziggurat_sampler.hpp"

#include <cmath>   // std::sqrt, std::log
#include <cstddef> // std::size_t
#include <limits>  // std::numeric_limits

namespace ropufu::aftermath::random
{
    /** @brief Ziggurat for normal distribution. */
    template <typename t_engine_type,
        typename t_value_type = typename probability::standard_normal_distribution<>::value_type,
        typename t_probability_type = typename probability::standard_normal_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename probability::standard_normal_distribution<t_value_type, t_probability_type>::expectation_type>
    struct normal_sampler_512;

    /** @brief Ziggurat for normal distribution. */
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct normal_sampler_512 :
        public ziggurat_sampler<
            normal_sampler_512<t_engine_type, t_value_type, t_probability_type, t_expectation_type>,
            t_engine_type,
            probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>,
            512>,
        public ziggurat_layers_normal_512<
            typename t_engine_type::result_type,
            t_value_type,
            t_expectation_type,
            t_engine_type::max() - t_engine_type::min()>
    {
        using type = normal_sampler_512<t_engine_type, t_value_type, t_probability_type, t_expectation_type>;

        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using distribution_type = probability::standard_normal_distribution<value_type, probability_type, expectation_type>;
        using ziggurat_type = ziggurat_sampler<type, engine_type, distribution_type, 512>;
        using uniform_type = typename engine_type::result_type;

        using result_type = value_type;
        static constexpr std::size_t n_boxes = 512;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();
        static constexpr long double scale = static_cast<long double>(engine_diameter) + 1;

        friend ziggurat_type;

        using ziggurat_type::ziggurat_sampler;

    protected:
        value_type sample_right_tail(engine_type& uniform_generator) noexcept
        {
            const value_type r = -type::layer_left_endpoints[1];
            const value_type r_squared = r * r;
            value_type result = 0;
            value_type u1, u2;
            while (true)
            {
                u1 = static_cast<value_type>((uniform_generator() - engine_type::min()) / type::scale);
                u2 = static_cast<value_type>((uniform_generator() - engine_type::min()) / type::scale);
                result = static_cast<value_type>(std::sqrt(r_squared - 2 * std::log(1 - u1)));
                if (u2 * result < r) return result;
            } // while(...)
        } // sample_right_tail(...)

        value_type sample_left_tail(engine_type& uniform_generator) noexcept
        {
            return -this->sample_right_tail(uniform_generator);
        } // sample_left_tail(...)
    }; // struct normal_sampler_512
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
