
#ifndef ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED

#include "../probability/normal_distribution.hpp"
#include "standard_normal_sampler_512.hpp"

namespace ropufu::aftermath::random
{
    /** @brief Sampler for Bernoulli distributions. */
    template <typename t_engine_type,
        typename t_value_type = typename probability::normal_distribution<>::value_type,
        typename t_probability_type = typename probability::normal_distribution<>::probability_type,
        typename t_expectation_type = typename probability::normal_distribution<t_probability_type>::expectation_type>
    struct normal_sampler_512;

    /** @brief Sampler for Normal distributions. */
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct normal_sampler_512
    {
        using type = normal_sampler_512<t_engine_type, t_value_type, t_probability_type, t_expectation_type>;

        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using distribution_type = probability::normal_distribution<value_type, probability_type, expectation_type>;
        using uniform_type = typename engine_type::result_type;

        using result_type = value_type;
        using standard_sampler_type = random::standard_normal_sampler_512<engine_type, value_type, probability_type, expectation_type>;

    private:
        standard_sampler_type m_standard_sampler = {};
        value_type m_shift = 0;
        value_type m_scale = 1;

    public:
        normal_sampler_512() noexcept
            : normal_sampler_512(distribution_type{})
        {
        } // normal_sampler_512(...)

        explicit normal_sampler_512(const distribution_type& dist) noexcept
            : m_shift(dist.mu()), m_scale(dist.sigma())
        {
        } // normal_sampler_512(...)

        // @todo Implement rejection probability upper bound.
        // /** Upper bound on the rejection probability. */
        // probability_type rejection_probability() const noexcept { return ??; }

        value_type sample(engine_type& uniform_generator) noexcept
        {
            return this->m_shift + this->m_scale * this->m_standard_sampler(uniform_generator);
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator) noexcept
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct normal_sampler_512
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
