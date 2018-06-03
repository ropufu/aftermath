
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED

#include "../probability/binomial_distribution.hpp"

#include <cmath>   // std::pow
#include <cstddef> // std::size_t
#include <forward_list> // std::forward_list
#include <vector>  // std::vector

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    struct sampler_binomial_alias
    {
        using type = sampler_binomial_alias<t_engine_type, t_result_type, t_param_type>;
        using engine_type = t_engine_type;
        using distribution_type = probability::binomial_distribution<t_result_type, t_param_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        result_type m_number_of_trials = 0;
        std::vector<result_type> m_alias = {};
        std::vector<param_type> m_cutoff = {};

    public:
        sampler_binomial_alias() noexcept : sampler_binomial_alias({}) { }

        explicit sampler_binomial_alias(const distribution_type& distribution) noexcept
            : m_number_of_trials(distribution.number_of_trials()), m_alias(distribution.number_of_trials() + 1), m_cutoff(distribution.number_of_trials() + 1)
        {
            result_type n = distribution.number_of_trials();
            param_type p = distribution.probability_of_success();

            std::vector<param_type> pmf(n + 1);
            std::forward_list<result_type> indices_big {};
            std::forward_list<result_type> indices_small {};

            pmf[0] = std::pow(1 - p, static_cast<param_type>(n));
            for (result_type k = 1; k <= n; ++k) pmf[k] = distribution.pdf(k);

            for (result_type k = 0; k <= n; ++k)
            {
                param_type z = (n + 1) * pmf[k];
                this->m_cutoff[k] = z;
                if (z >= 1) indices_big.push_front(k);
                else indices_small.push_front(k);
            } // for (...)

            while (!indices_big.empty() && !indices_small.empty())
            {
                result_type k = indices_big.front();   // m_cutoff[k] >= 1.
                result_type j = indices_small.front(); // m_cutoff[k] < 1.

                this->m_alias[j] = k;
                this->m_cutoff[k] -= (1 - this->m_cutoff[j]);

                indices_small.pop_front(); // Remove {j} from indices_small.
                if (this->m_cutoff[k] < 1)
                {
                    indices_big.pop_front(); // Remove {k} from indices_big.
                    indices_small.push_front(k); // Add {k} to indices_small.
                } // if (...)
            } // while(...)
        } // sampler_binomial_alias(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            param_type uniform_random = (uniform_generator() - engine_type::min()) / (static_cast<param_type>(type::diameter) + 1);

            param_type u = (this->m_number_of_trials + 1) * uniform_random; // uniform continuous \in[0, n + 1).
            result_type index = static_cast<result_type>(u); // uniform discrete \in[0, n].
            u = (index + 1) - u; // 1 - overshoot: uniform continuous \in(0, 1].
            return (u > this->m_cutoff[index]) ? this->m_alias[index] : index;
        } // operator ()(...)

        const std::vector<result_type>& alias() const noexcept { return this->m_alias; }

        const std::vector<param_type>& cutoff() const noexcept { return this->m_cutoff; }
    }; // struct sampler_binomial_alias
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_BINOMIAL_ALIAS_HPP_INCLUDED
