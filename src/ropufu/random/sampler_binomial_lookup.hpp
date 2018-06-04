
#ifndef ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "../probability/binomial_distribution.hpp"
#include "sampler_binomial_alias.hpp"

#include <cstddef>   // std::size_t
#include <cstring>   // std::memcpy
#include <forward_list> // std::forward_list
#include <stdexcept> // std::out_of_range, std::logic_error

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_param_type = double>
    struct sampler_binomial_lookup
    {
        using type = sampler_binomial_lookup<t_engine_type, t_result_type, t_param_type>;
        using engine_type = t_engine_type;
        using alias_type = sampler_binomial_alias<t_engine_type, t_result_type, t_param_type>;
        using distribution_type = probability::binomial_distribution<t_result_type, t_param_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        result_type m_number_of_trials_min = 1;
        result_type m_number_of_trials_max = 1;
        param_type m_probability_of_success = 0;
        //std::size_t m_current_row = 0;
        algebra::rmatrix_t<result_type> m_alias = {}; // Row-major storage; each row corresponds to the number of trials, column---position in the array.
        algebra::rmatrix_t<param_type> m_cutoff = {}; // Row-major storage; each row corresponds to the number of trials, column---position in the array.

    public:
        sampler_binomial_lookup() noexcept : sampler_binomial_lookup(1, 1, 0) { }

        /** @brief Constructs a lookup for a range of binomial distributions.
         *  @exception std::logic_error \p number_of_trials_min is greater than \number_of_trials_max.
         *  @exception std::out_of_range \p number_of_trials_min is zero.
         *  @exception std::out_of_range \p probability_of_success is negative or greater than one.
         */
        explicit sampler_binomial_lookup(result_type number_of_trials_min, result_type number_of_trials_max, param_type probability_of_success)
            : m_number_of_trials_min(number_of_trials_min), m_number_of_trials_max(number_of_trials_max), m_probability_of_success(probability_of_success),
            m_alias(number_of_trials_max - number_of_trials_min + 1, number_of_trials_max + 1), 
            m_cutoff(number_of_trials_max - number_of_trials_min + 1, number_of_trials_max + 1)
        {
            if (number_of_trials_min > number_of_trials_max) throw std::logic_error("<number_of_trials_min> must not exceed <number_of_trials_max>.");
            if (number_of_trials_min == 0) throw std::out_of_range("<number_of_trials_min> must be at least one.");
            if (probability_of_success < 0 || probability_of_success > 1) throw std::out_of_range("<probability_of_success> must be in the interval from 0 to 1.");

            std::size_t height = number_of_trials_max - number_of_trials_min + 1;
            // std::size_t width = number_of_trials_max + 1;
            for (result_type i = 0; i < height; ++i)
            {
                result_type n = number_of_trials_min + i;
                distribution_type distribution { n, probability_of_success };
                alias_type sampler { distribution };
                auto& alias = sampler.alias();
                auto& cutoff = sampler.cutoff();

                // Copy rows.
                std::memcpy(&this->m_alias(i, 0), alias.data(), alias.size() * sizeof(result_type)); // alias.size() is (n + 1)
                std::memcpy(&this->m_cutoff(i, 0), cutoff.data(), cutoff.size() * sizeof(param_type)); // cutoff.size() is (n + 1)
            } // for(...)
        }

        result_type number_of_trials_min() const noexcept { return this->m_number_of_trials_min; }
        result_type number_of_trials_max() const noexcept { return this->m_number_of_trials_max; }
        param_type probability_of_success() const noexcept { return this->m_probability_of_success; }

        result_type height() const noexcept { return this->m_number_of_trials_max - this->m_number_of_trials_min + 1; }
        result_type width() const noexcept { return this->m_number_of_trials_max + 1; }

        std::size_t size_in_bytes() const noexcept
        {
            result_type k = this->height() * this->width();
            return static_cast<std::size_t>(k) * (sizeof(result_type) + sizeof(param_type));
        } // size_in_bytes(...)

        result_type operator ()(result_type number_of_trials, engine_type& uniform_generator) const noexcept
        {
            result_type current_row = number_of_trials - this->m_number_of_trials_min;
            param_type uniform_random = (uniform_generator() - engine_type::min()) / (static_cast<param_type>(type::diameter) + 1);

            param_type u = (number_of_trials + 1) * uniform_random; // uniform continuous \in[0, n + 1).
            result_type index = static_cast<result_type>(u); // uniform discrete   \in[0, n].
            u = (index + 1) - u; // 1 - overshoot: uniform continuous \in(0, 1].
            return (u > this->m_cutoff(current_row, index)) ? this->m_alias(current_row, index) : index;
        } // operator ()(...)
    }; // struct sampler_binomial_lookup
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
