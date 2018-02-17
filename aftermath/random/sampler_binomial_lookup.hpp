
#ifndef ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "../math_constants.hpp"
#include "../not_an_error.hpp"
#include "../probability/dist_binomial.hpp"
#include "sampler_binomial_alias.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring> // for std::memcpy
#include <forward_list>
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            // Take advantage of consolidating storage when known upper and lower bounds on the number of trials exist.
            template <typename t_result_type, typename t_param_type, typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter>
            struct sampler_binomial_lookup
            {
                static constexpr t_bounds_type diameter = t_diameter;

                using type = sampler_binomial_lookup<t_result_type, t_param_type, t_uniform_type, t_bounds_type, t_diameter>;
                using alias_type = sampler_binomial_alias<t_result_type, t_param_type, t_uniform_type, t_bounds_type, t_diameter>;
                using distribution_type = probability::dist_binomial<t_result_type, t_param_type>;
                using result_type = typename distribution_type::result_type;
                using param_type = typename distribution_type::param_type;
                using uniform_type = t_uniform_type;
                using bounds_type = t_bounds_type;

            private:
                result_type m_number_of_trials_min;
                result_type m_number_of_trials_max;
                //std::size_t m_current_row = 0;
                algebra::matrix_row_major<result_type> m_alias; // Row-major storage; each row corresponds to the number of trials, column---position in the array.
                algebra::matrix_row_major<param_type> m_cutoff; // Row-major storage; each row corresponds to the number of trials, column---position in the array.

                void build(param_type probability_of_success) noexcept
                {
                    for (result_type i = 0; i < this->height(); ++i)
                    {
                        result_type n = this->m_number_of_trials_min + i;
                        distribution_type distribution(n, probability_of_success);
                        alias_type sampler(distribution);
                        auto& alias = sampler.alias();
                        auto& cutoff = sampler.cutoff();

                        std::memcpy(&this->m_alias.unchecked_at(i, 0), alias.data(), alias.size() * sizeof(result_type)); // alias.size() is (n + 1)
                        std::memcpy(&this->m_cutoff.unchecked_at(i, 0), cutoff.data(), cutoff.size() * sizeof(param_type)); // cutoff.size() is (n + 1)
                    }
                }

            public:
                sampler_binomial_lookup() noexcept
                    : sampler_binomial_lookup(distribution_type(), distribution_type())
                {
                }

                type& operator =(const type& other) noexcept
                {
                    if (this != &other)
                    {
                        this->m_number_of_trials_min = other.m_number_of_trials_min;
                        this->m_number_of_trials_max = other.m_number_of_trials_max;
                        this->m_alias = other.m_alias;
                        this->m_cutoff = other.m_cutoff;
                    }
                    return *this;
                }

                /** @brief Constructs a lookup for a range of binomial distributions.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if \p from and \p to have different probabilities of success.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the number of trials in \p from is greater than the number of trials in \p to.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if the number of trials in \p from is zero.
                 */
                explicit sampler_binomial_lookup(const distribution_type& from, const distribution_type& to) noexcept
                    : m_number_of_trials_min(from.number_of_trials()), m_number_of_trials_max(to.number_of_trials()),
                    m_alias(this->height(), this->width()), m_cutoff(this->height(), this->width())
                {
                    if (from.probability_of_success() != to.probability_of_success())
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "<from> and <to> must have the same probability of success.", __FUNCTION__, __LINE__);
                        return;
                    }
                    if (this->m_number_of_trials_min > this->m_number_of_trials_max)
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Number of trials in <to> must not be less than that in <from>.", __FUNCTION__, __LINE__);
                        return;
                    }
                    if (this->m_number_of_trials_min == 0)
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Number of trials in <from> must be at least one.", __FUNCTION__, __LINE__);
                        return;
                    }

                    // No need to initialize matrices, since they will be filled by build(...).
                    this->build(from.probability_of_success());
                }

                result_type number_of_trials_min() const noexcept { return this->m_number_of_trials_min; }

                result_type number_of_trials_max() const noexcept { return this->m_number_of_trials_max; }

                result_type height() const noexcept
                {
                    return this->m_number_of_trials_max - this->m_number_of_trials_min + 1;
                }

                result_type width() const noexcept
                {
                    return this->m_number_of_trials_max + 1;
                }

                std::size_t size_in_bytes() const noexcept
                {
                    std::size_t k = this->height() * this->width();
                    return k * (sizeof(result_type) + sizeof(param_type));
                }
                
                //// Set the number of trials used in the generator; should be between <t_number_of_trials_min> and <t_number_of_trials_max>.
                //void at(result_type number_of_trials)
                //{
                //    if (number_of_trials < this->m_number_of_trials_min || number_of_trials > this->m_number_of_trials_max)
                //        throw std::out_of_range("<number_of_trials> should be between <number_of_trials_min> and <number_of_trials_max>.");
                //    this->m_current_row = number_of_trials - this->m_number_of_trials_min;
                //}

                template <typename t_engine_type>
                result_type operator ()(result_type number_of_trials, t_engine_type& uniform_generator) const noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    result_type current_row = number_of_trials - this->m_number_of_trials_min;
                    param_type uniform_random = (uniform_generator() - t_engine_type::min()) / (static_cast<param_type>(type::diameter) + 1);

                    param_type u = (number_of_trials + 1) * uniform_random; // uniform continuous \in[0, n + 1).
                    result_type index = static_cast<result_type>(u);    // uniform discrete   \in[0, n].
                    u = (index + 1) - u;                                // 1 - overshoot: uniform continuous \in(0, 1].
                    return (u > this->m_cutoff.unchecked_at(current_row, index)) ? this->m_alias.unchecked_at(current_row, index) : index;
                }
            };

            template <typename t_engine_type, typename t_param_type = double>
            using default_sampler_binomial_lookup_t = sampler_binomial_lookup<std::size_t, t_param_type, typename t_engine_type::result_type, std::size_t, t_engine_type::max() - t_engine_type::min()>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
