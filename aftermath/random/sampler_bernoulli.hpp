
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED

#include "../probability/dist_binomial.hpp"

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
            struct sampler_bernoulli
            {
                static const t_bounds_type diameter = t_diameter;

                using type = sampler_bernoulli<t_uniform_type, t_bounds_type, diameter>;
                using distribution_type = probability::dist_binomial;
                using result_type = distribution_type::result_type;
                using uniform_type = t_uniform_type;
                using bounds_type = t_bounds_type;

            private:
                std::size_t m_number_of_trials;
                uniform_type m_threshold;

            public:
                sampler_bernoulli() noexcept
                    : sampler_bernoulli(distribution_type())
                {
                }

                type& operator =(const type& other) noexcept
                {
                    if (this != &other)
                    {
                        this->m_number_of_trials = other.m_number_of_trials;
                        this->m_threshold = other.m_threshold;
                    }
                    return *this;
                }

                explicit sampler_bernoulli(const distribution_type& distribution) noexcept
                    : m_number_of_trials(distribution.number_of_trials()),
                    m_threshold(static_cast<uniform_type>(type::diameter * distribution.probability_of_success()))
                {
                }

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator) const noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    std::size_t count_success = std::size_t();
                    for (std::size_t i = std::size_t(); i < this->m_number_of_trials; i++) if ((uniform_generator() - t_engine_type::min()) <= this->m_threshold) count_success++;
                    return count_success;
                }
            };

            template <typename t_engine_type>
            using default_sampler_bernoulli_t = sampler_bernoulli<typename t_engine_type::result_type, std::size_t, t_engine_type::max() - t_engine_type::min()>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED
