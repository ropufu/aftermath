
#ifndef ROPUFU_AFTERMATH_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED

#include "../probability/uniform_int_distribution.hpp"

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    /** @brief Accurate rejection sampler for discrete uniform distributions. */
    template <typename t_engine_type,
        typename t_value_type = typename probability::uniform_int_distribution<>::value_type,
        typename t_probability_type = typename probability::uniform_int_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename probability::uniform_int_distribution<t_value_type, t_probability_type>::expectation_type,
        std::size_t t_rejection_loop_max = 100>
    struct uniform_int_sampler;

    /** @brief Accurate rejection sampler for discrete uniform distributions. */
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, std::size_t t_rejection_loop_max>
    struct uniform_int_sampler
    {
        using type = uniform_int_sampler<t_engine_type, t_value_type, t_probability_type, t_expectation_type, t_rejection_loop_max>;

        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using distribution_type = probability::uniform_int_distribution<value_type, probability_type, expectation_type>;
        using uniform_type = typename engine_type::result_type;

        using result_type = value_type;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();
        static constexpr std::size_t rejection_loop_max = t_rejection_loop_max;

    private:
        value_type m_min = 0;
        uniform_type m_diameter = type::engine_diameter;
        uniform_type m_engine_blocks = 1;

    public:
        uniform_int_sampler()
            : uniform_int_sampler(distribution_type{})
        {
        } // uniform_int_sampler(...)

        explicit uniform_int_sampler(const distribution_type& distribution)
            : m_min(distribution.a()), m_diameter(static_cast<uniform_type>(distribution.b() - distribution.a()))
        {
            if (type::engine_diameter < this->m_diameter) throw std::overflow_error("Engine diameter insufficient for the distribution.");
            if (this->m_diameter + 1 < this->m_diameter) throw std::overflow_error("Engine precision insufficient for the distribution.");
            this->m_engine_blocks = 1 + (type::engine_diameter - this->m_diameter) / (this->m_diameter + 1);

            if (this->m_engine_blocks < 1) // Overflow: number of engine blocks cannot be stored.
            {
                this->m_engine_blocks = type::engine_diameter;
                this->m_diameter = 0; // Resort to rejection when the maximum value is generated.
            } // if (...)
        } // uniform_int_sampler(...)

        /** Upper bound on the probability of error. */
        constexpr probability_type error_probability() const noexcept { return 0; }

        /** Upper bound on the rejection probability. */
        probability_type rejection_probability() const noexcept
        {
            uniform_type fractional_block_size = (type::engine_diameter - this->m_diameter) % (this->m_diameter + 1);
            // For exact rejection probability the denominator on the next line has to be (type::engine_diameter + 1).
            return static_cast<probability_type>(fractional_block_size) / type::engine_diameter;
        } // rejection_probability(...)

        /** @warning This function utilizes a rejection sampling algorithm.
         *  If \p uniform_generator is not configured properly, this could lead to a \c std::runtime_error.
         *  @exception std::runtime_error Rejection sampling exceeded maximum allowed attempts.
         */
        value_type sample(engine_type& uniform_generator)
        {
            std::size_t count_rejections = 0; // Prevent infinite loops for "bad" uniform generators.

            uniform_type u = uniform_generator() - engine_type::min(); // Uniform over {0, ..., engine_diameter}.
            uniform_type k = u / this->m_engine_blocks; // Conditionally uniform {0, ..., b - a}, given k <= (b - a).
            while (k > this->m_diameter) // Reject if we are in the fractional block.
            {
                ++count_rejections;
                if (count_rejections > type::rejection_loop_max)
                    throw std::runtime_error("Sampler exceeded maximum allowed rejections.");

                u = uniform_generator() - engine_type::min();
                k = u / this->m_engine_blocks;
            } // while (...)
            return this->m_min + static_cast<value_type>(k);
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator)
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct uniform_int_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED
