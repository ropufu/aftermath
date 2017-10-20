
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../probability/dist_binomial.hpp"
#include "../template_math.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <forward_list>
#include <type_traits>
#include <vector>

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            template <typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter>
            struct sampler_binomial_alias
            {
                static const t_bounds_type diameter = t_diameter;

                using type = sampler_binomial_alias<t_uniform_type, t_bounds_type, diameter>;
                using distribution_type = probability::dist_binomial;
                using result_type = distribution_type::result_type;
                using uniform_type = t_uniform_type;
                using bounds_type = t_bounds_type;

            private:
                //distribution_type m_distribution;
                std::size_t m_number_of_trials;
                std::vector<result_type> m_alias;
                std::vector<double> m_cutoff;

            public:
                sampler_binomial_alias() noexcept
                    : sampler_binomial_alias(distribution_type())
                {
                }

                type& operator =(const type& other) noexcept
                {
                    if (this != &other)
                    {
                        this->m_number_of_trials = other.m_number_of_trials;
                        this->m_alias = other.m_alias;
                        this->m_cutoff = other.m_cutoff;
                    }
                    return *this;
                }

                explicit sampler_binomial_alias(const distribution_type& distribution) noexcept
                    : m_number_of_trials(distribution.number_of_trials()), m_alias(distribution.number_of_trials() + 1), m_cutoff(distribution.number_of_trials() + 1)
                {
                    auto n = distribution.number_of_trials();
                    auto p = distribution.probability_of_success();

                    std::vector<double> pmf(n + 1);
                    std::forward_list<result_type> indices_big;
                    std::forward_list<result_type> indices_small;

                    pmf[0] = std::pow(1 - p, n);
                    for (result_type k = 1; k <= n; k++) pmf[k] = distribution.pdf(k);

                    for (result_type k = 0; k <= n; k++)
                    {
                        double z = (n + 1) * pmf[k];
                        this->m_cutoff[k] = z;
                        if (z >= 1.0) indices_big.push_front(k);
                        else indices_small.push_front(k);
                    }

                    while (!indices_big.empty() && !indices_small.empty())
                    {
                        result_type k = indices_big.front();   // m_cutoff[k] >= 1.
                        result_type j = indices_small.front(); // m_cutoff[k] < 1.

                        this->m_alias[j] = k;
                        this->m_cutoff[k] -= 1 - this->m_cutoff[j];

                        indices_small.pop_front(); // Remove {j} from indices_small.
                        if (this->m_cutoff[k] < 1.0)
                        {
                            indices_big.pop_front(); // Remove {k} from indices_big.
                            indices_small.push_front(k); // Add {k} to indices_small.
                        }
                    }
                }

                template <typename t_engine_type>
                result_type operator ()(t_engine_type& uniform_generator) const noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    double uniform_random = (uniform_generator() - t_engine_type::min()) / (static_cast<double>(type::diameter) + 1);

                    double u = (this->m_number_of_trials + 1) * uniform_random; // uniform continuous \in[0, n + 1).
                    result_type index = static_cast<result_type>(u);            // uniform discrete   \in[0, n].
                    u = (index + 1) - u;                                        // 1 - overshoot: uniform continuous \in(0, 1].
                    return (u > this->m_cutoff[index]) ? this->m_alias[index] : index;
                }

                const std::vector<result_type>& alias() const noexcept
                {
                    return this->m_alias;
                }

                const std::vector<double>& cutoff() const noexcept
                {
                    return this->m_cutoff;
                }
            };

            template <typename t_engine_type>
            using default_sampler_binomial_t = sampler_binomial_alias<typename t_engine_type::result_type, std::size_t, t_engine_type::max() - t_engine_type::min()>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED
