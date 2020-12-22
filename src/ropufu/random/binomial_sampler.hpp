
#ifndef ROPUFU_AFTERMATH_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED

#include "../probability/binomial_distribution.hpp"
#include "../rationalize.hpp"

namespace ropufu::aftermath::random
{
    /** @brief Sampler for Bernoulli distributions. */
    template <typename t_engine_type,
        typename t_value_type = typename probability::binomial_distribution<>::value_type,
        typename t_probability_type = typename probability::binomial_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename probability::binomial_distribution<t_value_type, t_probability_type>::expectation_type>
    struct binomial_sampler;

    /** @brief Sampler for Bernoulli distributions. */
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct binomial_sampler
    {
        using type = binomial_sampler<t_engine_type, t_value_type, t_probability_type, t_expectation_type>;

        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using distribution_type = probability::binomial_distribution<value_type, probability_type, expectation_type>;
        using uniform_type = typename engine_type::result_type;

        using result_type = value_type;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();

    private:
        using rationalize_t = rationalize<probability_type, uniform_type, type::engine_diameter>;

        value_type m_offset = 0;
        value_type m_count = 0;
        value_type m_effective_count = 0;
        uniform_type m_threshold = engine_type::min();

    public:
        binomial_sampler() noexcept
            : binomial_sampler(distribution_type{})
        {
        } // binomial_sampler(...)

        explicit binomial_sampler(const distribution_type& dist) noexcept
            : m_count(dist.number_of_trials()), m_effective_count(dist.number_of_trials())
        {
            uniform_type h = rationalize_t::probability(dist.probability_of_success());
            this->m_threshold = engine_type::min() + h;

            // ~~ Special case p* = 0: always generate \c false. ~~
            if (h == 0) this->m_effective_count = 0;
            // ~~ Special case p = 1: always generate \c true. ~~
            if (dist.probability_of_success() == 1)
            {
                this->m_offset = dist.number_of_trials();
                this->m_effective_count = 0;
            } // if (...)
        } // binomial_sampler(...)

        /** Upper bound on the probability of error. */
        constexpr probability_type error_probability() const noexcept
        {
            return static_cast<probability_type>(this->m_count) / static_cast<probability_type>(type::engine_diameter);
        } // error_probability(...)

        /** Upper bound on the rejection probability. */
        constexpr probability_type rejection_probability() const noexcept { return 0; }

        value_type sample(engine_type& uniform_generator) noexcept
        {
            value_type result = this->m_offset;
            for (value_type k = 0; k < this->m_effective_count; ++k)
            {
                result += (uniform_generator() < this->m_threshold) ? 1 : 0;
            } // for (...)
            return result;
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator) noexcept
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct binomial_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED
