
#ifndef ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED

#include "../probability/bernoulli_distribution.hpp"

namespace ropufu::aftermath::random
{
    /** @brief Sampler for Bernoulli distribution. */
    template <typename t_engine_type,
        typename t_probability_type = typename probability::bernoulli_distribution<>::probability_type,
        typename t_expectation_type = typename probability::bernoulli_distribution<t_probability_type>::expectation_type>
    struct bernoulli_sampler;

    /** @brief Sampler for Bernoulli distribution.
     *  @todo Add tests!!
     */
    template <typename t_engine_type, typename t_probability_type, typename t_expectation_type>
    struct bernoulli_sampler
    {
        using type = bernoulli_sampler<t_engine_type, t_probability_type, t_expectation_type>;

        using engine_type = t_engine_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using distribution_type = probability::bernoulli_distribution<probability_type, expectation_type>;
        using value_type = typename distribution_type::value_type;
        using uniform_type = typename engine_type::result_type;

        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();
        static constexpr long double scale = static_cast<long double>(engine_diameter) + 1;

    private:
        value_type m_always_true = false;
        uniform_type m_threshold = 0;

    public:
        bernoulli_sampler() noexcept
            : bernoulli_sampler(distribution_type{})
        {
        } // bernoulli_sampler(...)

        explicit bernoulli_sampler(const distribution_type& distribution) noexcept
            : m_threshold(static_cast<uniform_type>(0.5L + type::scale * distribution.probability_of_success()))
        {
            static constexpr long double potential_overshoot = static_cast<long double>(engine_diameter) + 0.5L;
            long double p_upscaled = type::scale * static_cast<long double>(distribution.probability_of_success());
            if (p_upscaled >= potential_overshoot) this->m_always_true = true;
        } // bernoulli_sampler(...)

        value_type sample(engine_type& uniform_generator) noexcept
        {
            return (this->m_always_true | ((uniform_generator() - engine_type::min()) < this->m_threshold));
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator) noexcept
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct bernoulli_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
