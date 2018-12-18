
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_LOOKUP_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_LOOKUP_HPP_INCLUDED

#include "../on_error.hpp"
#include "../algebra/matrix.hpp"
#include "../probability/binomial_distribution.hpp"
#include "binomial_alias_sampler.hpp"

#include <cstddef>   // std::size_t, std::nullptr_t
#include <cstring>   // std::memcpy
#include <forward_list> // std::forward_list
#include <system_error> // std::error_code, std::errc

namespace ropufu::aftermath::random
{
    template <typename t_engine_type, typename t_result_type = std::size_t, typename t_probability_type = double>
    struct binomial_lookup
    {
        using type = binomial_lookup<t_engine_type, t_result_type, t_probability_type>;
        using alias_type = binomial_alias_sampler<t_engine_type, t_result_type, t_probability_type>;

        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;

        using distribution_type = probability::binomial_distribution<t_result_type, t_probability_type>;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        static constexpr expectation_type norm = static_cast<expectation_type>(type::diameter) + 1;

    private:
        result_type m_min = 0;
        result_type m_max = 0;
        probability_type m_probability_of_success = 0;
        //std::size_t m_current_row = 0;
        algebra::rmatrix_t<result_type> m_alias = algebra::rmatrix_t<result_type>(1, 1); // Row-major storage; each row corresponds to the number of trials, column---position in the array.
        algebra::rmatrix_t<expectation_type> m_cutoff = algebra::rmatrix_t<expectation_type>(1, 1); // Row-major storage; each row corresponds to the number of trials, column---position in the array.

    public:
        binomial_lookup() noexcept { }

        /** @brief Constructs a lookup for a range of binomial distributions.
         *  @param ec Set to \c std::errc::invalid_argument if the number of trials in \p from is greater than the number of trials in \p to.
         *  @param ec Set to \c std::errc::invalid_argument if probability of success is different for \p from and \p to.
         */
        binomial_lookup(const distribution_type& from, const distribution_type& to, std::error_code& ec) noexcept
        {
            if (from.number_of_trials() > to.number_of_trials()) aftermath::detail::on_error(ec, std::errc::invalid_argument, "Number of trials in <from> must not exceed the number of trials in <to>.");
            if (from.probability_of_success() != to.probability_of_success()) aftermath::detail::on_error(ec, std::errc::invalid_argument, "<probability_of_success> must be the same for both <from> and <to>.");
            if (ec) return;
            
            this->m_min = from.number_of_trials();
            this->m_max = to.number_of_trials();
            this->m_probability_of_success = from.probability_of_success();

            this->m_alias = algebra::rmatrix_t<result_type>(this->m_max - this->m_min + 1, this->m_max + 1);
            this->m_cutoff = algebra::rmatrix_t<expectation_type>(this->m_max - this->m_min + 1, this->m_max + 1);

            result_type height = this->m_max - this->m_min + 1;
            // result_type width = this->m_max + 1;
            for (result_type i = 0; i < height; ++i)
            {
                result_type n = this->m_min + i;
                distribution_type distribution { n, this->m_probability_of_success, ec };
                if (ec) return;
                alias_type sampler { distribution };
                const std::vector<result_type>& alias = sampler.alias();
                const std::vector<expectation_type>& cutoff = sampler.cutoff();

                // Copy rows.
                std::memcpy(&this->m_alias(i, 0), alias.data(), alias.size() * sizeof(result_type)); // alias.size() is (n + 1)
                std::memcpy(&this->m_cutoff(i, 0), cutoff.data(), cutoff.size() * sizeof(expectation_type)); // cutoff.size() is (n + 1)
            } // for(...)
        } // binomial_lookup(...)

        result_type min() const noexcept { return this->m_min; }
        result_type max() const noexcept { return this->m_max; }
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }

        result_type height() const noexcept { return this->m_max - this->m_min + 1; }
        result_type width() const noexcept { return this->m_max + 1; }

        std::size_t size_in_bytes() const noexcept
        {
            result_type k = this->height() * this->width();
            return static_cast<std::size_t>(k) * (sizeof(result_type) + sizeof(expectation_type));
        } // size_in_bytes(...)

        /** @warning This operator does not check if \p number_of_trials lies between \c min() and \c max(). */
        result_type operator ()(result_type number_of_trials, engine_type& uniform_generator) const
        {
            result_type current_row = number_of_trials - this->m_min; // Potential overflow.
            expectation_type uniform_random = static_cast<expectation_type>(uniform_generator() - engine_type::min()) / (type::norm);

            expectation_type u = static_cast<expectation_type>((number_of_trials + 1) * uniform_random); // uniform continuous \in[0, n + 1).
            result_type index = static_cast<result_type>(u); // uniform discrete \in[0, n].
            u = (index + 1) - u; // 1 - overshoot: uniform continuous \in(0, 1].
            expectation_type cutoff = this->m_cutoff(current_row, index);
            result_type maybe = this->m_alias(current_row, index);
            return (u > cutoff) ? maybe : index; // Potential unauthorized memory access due to unchecked <number_of_trials>.
        } // operator ()(...)
    }; // struct binomial_lookup
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_LOOKUP_HPP_INCLUDED
