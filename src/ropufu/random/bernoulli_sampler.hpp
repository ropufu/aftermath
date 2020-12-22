
#ifndef ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED

#include "../probability/bernoulli_distribution.hpp"
#include "../rationalize.hpp"

namespace ropufu::aftermath::random
{
    /** @brief Sampler for Bernoulli distributions. */
    template <typename t_engine_type,
        typename t_probability_type = typename probability::bernoulli_distribution<>::probability_type,
        typename t_expectation_type = typename probability::bernoulli_distribution<t_probability_type>::expectation_type>
    struct bernoulli_sampler;

    /** @brief Sampler for Bernoulli distributions. */
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

        using result_type = value_type;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();

    private:
        using rationalize_t = rationalize<probability_type, uniform_type, type::engine_diameter>;

        uniform_type m_factor = 1;
        uniform_type m_threshold = engine_type::min();

    public:
        bernoulli_sampler() noexcept
            : bernoulli_sampler(distribution_type{})
        {
        } // bernoulli_sampler(...)

        explicit bernoulli_sampler(const distribution_type& dist) noexcept
        {
            uniform_type h = rationalize_t::probability(dist.probability_of_success());
            this->m_threshold = engine_type::min() + h;

            // ~~ Special case p = 1: always generate \c true. ~~
            if (dist.probability_of_success() == 1)
            {
                this->m_factor = 0;
                this->m_threshold = 1;
            } // if (...)
        } // bernoulli_sampler(...)

        /** Upper bound on the probability of error. */
        constexpr probability_type error_probability() const noexcept { return 1 / static_cast<probability_type>(type::engine_diameter); }

        /** Upper bound on the rejection probability. */
        constexpr probability_type rejection_probability() const noexcept { return 0; }

        value_type sample(engine_type& uniform_generator) noexcept
        {
            return (this->m_factor * uniform_generator()) < this->m_threshold;
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator) noexcept
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct bernoulli_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
