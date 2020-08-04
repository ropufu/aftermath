
#ifndef ROPUFU_AFTERMATH_RANDOM_ALIAS_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ALIAS_SAMPLER_HPP_INCLUDED

#include "../probability/concepts.hpp"
#include "../rationalize.hpp"
#include "uniform_int_sampler.hpp"

#include <cstddef>   // std::size_t
#include <stdexcept> // std::logic_error
#include <vector>    // std::vector

namespace ropufu::aftermath::random
{
    template <typename t_engine_type,
        ropufu::distribution t_distribution_type,
        typename t_index_sampler_type = uniform_int_sampler<
            t_engine_type,
            std::size_t,
            typename t_distribution_type::probability_type,
            typename t_distribution_type::expectation_type>>
        requires probability::is_discrete_v<t_distribution_type> && probability::has_bounded_support_v<t_distribution_type>
    struct alias_sampler;
    
    template <typename t_engine_type, ropufu::distribution t_distribution_type, typename t_index_sampler_type>
        requires probability::is_discrete_v<t_distribution_type> && probability::has_bounded_support_v<t_distribution_type>
    struct alias_sampler
    {
        using type = alias_sampler<t_engine_type, t_distribution_type, t_index_sampler_type>;

        using engine_type = t_engine_type;
        using distribution_type = t_distribution_type;
        using index_sampler_type = t_index_sampler_type;

        using value_type = typename distribution_type::value_type;
        using probability_type = typename distribution_type::probability_type;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename engine_type::result_type;

        using result_type = value_type;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();

    private:
        using rationalize_t = rationalize<probability_type, uniform_type, type::engine_diameter>;

        std::vector<value_type> m_support = {};
        std::vector<value_type> m_alias = {};
        std::vector<uniform_type> m_cutoff = {};
        index_sampler_type m_index_sampler = {};

    public:
        alias_sampler()
            : alias_sampler(distribution_type{})
        {
        } // alias_sampler(...)

        /** @brief Constructs an alias sampler for \p distribution.
         *  @exception std::logic_error Trivial distributions not supported.
         *  @exception std::logic_error \c t_engine_type cannot accomodate such a wide distribution.
         */
        explicit alias_sampler(const distribution_type& distribution)
            : m_support(distribution.support()),
            m_alias(this->m_support),
            m_cutoff(this->m_support.size())
        {
            std::size_t n = this->m_support.size();
            if (n == 0) throw std::logic_error("Trivial distributions not supported.");
            if (n - 1 > type::engine_diameter) throw std::logic_error("Engine cannot accomodate such a wide distribution.");
            this->m_index_sampler = index_sampler_type(typename index_sampler_type::distribution_type(0, n - 1));

            probability_type p_scale = static_cast<probability_type>(n);

            std::vector<probability_type> upscaled_pmf {};
            std::vector<std::size_t> indices_small {};
            std::vector<std::size_t> indices_big {};

            upscaled_pmf.reserve(n);
            indices_small.reserve(n);
            indices_big.reserve(n);

            // Cache the p.m.f.
            for (std::size_t i = 0; i < n; ++i)
            {
                const value_type& x = this->m_support[i];
                probability_type np = distribution.pmf(x, p_scale);
                upscaled_pmf.push_back(np);
            } // for (...)

            // Classify indices.
            for (std::size_t i = 0; i < n; ++i)
            {
                const probability_type& np = upscaled_pmf[i];
                if (np <= 1) indices_small.push_back(i);
                else indices_big.push_back(i);
            } // for (...)

            // Distribute "large" probabilities among aliases of "small" branches.
            while (!indices_big.empty() && !indices_small.empty())
            {
                std::size_t j = indices_small.back(); // Index of the last small element.
                std::size_t k = indices_big.back();   // Index of the last large element
                this->m_alias[j] = this->m_support[k]; // Use large element as an alias for the small element.

                const probability_type& np = upscaled_pmf[j];
                this->m_cutoff[j] = rationalize_t::probability(np);
                if (np == 1) this->m_alias[j] = this->m_support[j]; // Treat almost sure events as certain.

                probability_type delta = 1 - np; // Probability of small item not being selected...
                upscaled_pmf[k] -= delta; // ...is reassigned to the probability of its alias being chosen.

                indices_small.pop_back(); // Remove {j} from indices_small.
                if (upscaled_pmf[k] <= 1)
                {
                    indices_big.pop_back(); // Remove {k} from indices_big.
                    indices_small.push_back(k); // Add {k} to indices_small.
                } // if (...)
            } // while(...)

            // Take care of rounding errors.
            for (std::size_t j : indices_small) this->m_alias[j] = this->m_support[j];
            for (std::size_t k : indices_big) this->m_alias[k] = this->m_support[k];

            // Adjust thresholds for generators that have non-zero min.
            if constexpr (engine_type::min() != 0)
                for (uniform_type& x : this->m_cutoff) x += engine_type::min();
        } // alias_sampler(...)

        const std::vector<value_type>& support() const noexcept { return this->m_support; }
        const std::vector<value_type>& alias() const noexcept { return this->m_alias; }
        const std::vector<uniform_type>& cutoff() const noexcept { return this->m_cutoff; }

        const index_sampler_type& index_sampler() const noexcept { return this->m_index_sampler; }

        value_type sample(engine_type& uniform_generator)
        {
            // ~~ Stage 1 (index): generate a discrete uniform over {0, 1, ..., n - 1}. ~~
            std::size_t index = this->m_index_sampler.sample(uniform_generator);
            // ~~ Stage 2 (cutoff): generate a discrete uniform over {0, 1, ..., engine_diameter}. ~~
            uniform_type u = uniform_generator();
            return (u < this->m_cutoff[index]) ? this->m_support[index] : this->m_alias[index];
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator)
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct alias_sampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_ALIAS_SAMPLER_HPP_INCLUDED
