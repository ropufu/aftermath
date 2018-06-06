
#ifndef ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED

#include "../probability/binomial_distribution.hpp"

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_probability_type = double>
    struct bernoulli_sampler
    {
        using type = bernoulli_sampler<t_engine_type, t_result_type, t_probability_type>;

        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;
        
        using distribution_type = probability::binomial_distribution<t_result_type, t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        static constexpr expectation_type norm = static_cast<expectation_type>(type::diameter) + 1;

    private:
        result_type m_number_of_trials = 0;
        uniform_type m_threshold = 0;

    public:
        bernoulli_sampler() noexcept { }

        explicit bernoulli_sampler(const distribution_type& distribution) noexcept
            : m_number_of_trials(distribution.number_of_trials()),
            m_threshold(static_cast<uniform_type>(type::diameter * distribution.probability_of_success()))
        {
        } // bernoulli_sampler(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            result_type count_success = 0;
            for (result_type i = 0; i < this->m_number_of_trials; ++i) if ((uniform_generator() - engine_type::min()) <= this->m_threshold) ++count_success;
            return count_success;
        } // operator ()(...)
    }; // struct bernoulli_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
