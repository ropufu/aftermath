
#ifndef ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED

#include "../probability/binomial_distribution.hpp"

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    struct sampler_bernoulli
    {
        using type = sampler_bernoulli<t_engine_type, t_param_type, t_result_type>;
        using engine_type = t_engine_type;
        using distribution_type = probability::binomial_distribution<t_result_type, t_param_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        result_type m_number_of_trials = 0;
        uniform_type m_threshold = 0;

    public:
        sampler_bernoulli() noexcept : sampler_bernoulli({}) { }

        explicit sampler_bernoulli(const distribution_type& distribution) noexcept
            : m_number_of_trials(distribution.number_of_trials()),
            m_threshold(static_cast<uniform_type>(type::diameter * distribution.probability_of_success()))
        {
        } // sampler_bernoulli(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            result_type count_success = 0;
            for (result_type i = 0; i < this->m_number_of_trials; ++i) if ((uniform_generator() - t_engine_type::min()) <= this->m_threshold) ++count_success;
            return count_success;
        } // operator ()(...)
    }; // struct sampler_bernoulli
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_SAMPLER_BERNOULLI_HPP_INCLUDED
