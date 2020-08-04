
#ifndef ROPUFU_AFTERMATH_RANDOM_ALIAS_MULTISAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ALIAS_MULTISAMPLER_HPP_INCLUDED

#include "../probability/concepts.hpp"
#include "alias_sampler.hpp"
#include "uniform_int_sampler.hpp"

#include <algorithm> // std::copy
#include <cstddef>   // std::size_t
#include <iterator>  // std::back_inserter
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
    struct alias_multisampler;
    
    template <typename t_engine_type, ropufu::distribution t_distribution_type, typename t_index_sampler_type>
        requires probability::is_discrete_v<t_distribution_type> && probability::has_bounded_support_v<t_distribution_type>
    struct alias_multisampler
    {
        using type = alias_multisampler<t_engine_type, t_distribution_type, t_index_sampler_type>;
        using alias_type = alias_sampler<t_engine_type, t_distribution_type, t_index_sampler_type>;

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

        // ~~ Condensed alias fields. ~~
        std::vector<value_type> m_support = {};
        std::vector<value_type> m_alias = {};
        std::vector<uniform_type> m_cutoff = {};
        // ~~ Samplers for each distribution in the collection. ~~
        std::vector<index_sampler_type> m_index_samplers = {};
        // ~~ Offset indices for each of the distributions. ~~
        std::vector<std::size_t> m_offset_indices = {};

    public:
        alias_multisampler() noexcept { }

        void reserve(std::size_t count_distributions, std::size_t count_total_support_size)
        {
            this->m_support.reserve(count_total_support_size);
            this->m_alias.reserve(count_total_support_size);
            this->m_cutoff.reserve(count_total_support_size);

            this->m_index_samplers.reserve(count_distributions);
            this->m_offset_indices.reserve(count_distributions);
        } // reserve(...)

        std::size_t push_back(const distribution_type& distribution)
        {
            alias_type a { distribution };
            std::size_t offset_index = this->m_support.size();

            std::copy(a.support().begin(), a.support().end(), std::back_inserter(this->m_support));
            std::copy(a.alias().begin(), a.alias().end(), std::back_inserter(this->m_alias));
            std::copy(a.cutoff().begin(), a.cutoff().end(), std::back_inserter(this->m_cutoff));

            this->m_index_samplers.push_back(a.index_sampler());
            this->m_offset_indices.push_back(offset_index);

            return this->m_offset_indices.size() - 1;
        } // push_back(...)

        value_type sample(std::size_t distribution_index, engine_type& uniform_generator)
        {
            index_sampler_type& index_sampler = this->m_index_samplers[distribution_index];
            std::size_t index_offset = this->m_offset_indices[distribution_index];
            
            // ~~ Stage 1 (index): generate a discrete uniform over {0, 1, ..., n - 1}. ~~
            std::size_t index = index_sampler.sample(uniform_generator) + index_offset;
            // ~~ Stage 2 (cutoff): generate a discrete uniform over {0, 1, ..., engine_diameter}. ~~
            uniform_type u = uniform_generator();
            return (u < this->m_cutoff[index]) ? this->m_support[index] : this->m_alias[index];
        } // sample(...)

        value_type operator ()(std::size_t distribution_index, engine_type& uniform_generator)
        {
            return this->sample(distribution_index, uniform_generator);
        } // operator ()(...)
    }; // struct alias_multisampler
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_ALIAS_MULTISAMPLER_HPP_INCLUDED
