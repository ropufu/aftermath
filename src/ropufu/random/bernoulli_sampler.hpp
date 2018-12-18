
#ifndef ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED

#include "../probability/bernoulli_distribution.hpp"

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_probability_type = double>
    struct bernoulli_sampler
    {
        using type = bernoulli_sampler<t_engine_type, t_probability_type>;

        using engine_type = t_engine_type;
        using result_type = bool;
        using probability_type = t_probability_type;
        
        using distribution_type = probability::bernoulli_distribution<t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        static constexpr expectation_type norm = static_cast<expectation_type>(type::diameter) + 1;

    private:
        expectation_type m_threshold = 0;

    public:
        bernoulli_sampler() noexcept { }

        explicit bernoulli_sampler(const distribution_type& distribution) noexcept
            : m_threshold(static_cast<expectation_type>(type::norm * distribution.probability_of_success()))
        {
        } // bernoulli_sampler(...)

        result_type operator ()(engine_type& uniform_generator) const noexcept
        {
            return static_cast<expectation_type>((uniform_generator() - engine_type::min()) < this->m_threshold);
        } // operator ()(...)
    }; // struct bernoulli_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
