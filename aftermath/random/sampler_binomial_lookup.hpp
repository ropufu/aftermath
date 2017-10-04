
#ifndef ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "../math_constants.hpp"
#include "../probability/dist_binomial.hpp"
#include "sampler_binomial_alias.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring> // for std::memcpy
#include <forward_list>
#include <stdexcept> // for std::out_of_range
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            // Take advantage of consolidating storage when known upper and lower bounds on the number of trials exist.
            template <typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter>
            struct sampler_binomial_lookup
            {
                static const t_bounds_type diameter = t_diameter;

                typedef probability::dist_binomial     distribution_type;
                typedef distribution_type::result_type result_type;
                typedef t_uniform_type                 uniform_type;
                typedef t_bounds_type                  bounds_type;
                typedef sampler_binomial_lookup<uniform_type, bounds_type, diameter> type;

            private:
                std::size_t m_number_of_trials_min;
                std::size_t m_number_of_trials_max;
                //std::size_t m_current_row = std::size_t();
                algebra::matrix_column_major<result_type> m_alias; // Column-major storage; each row corresponds to the number of trials, column---position in the array.
                algebra::matrix_column_major<double> m_cutoff; // Column-major storage; each row corresponds to the number of trials, column---position in the array.

                void build(double probability_of_success)
                {
                    for (std::size_t i = 0UL; i < this->height(); i++)
                    {
                        std::size_t n = this->m_number_of_trials_min + i;
                        distribution_type distribution(n, probability_of_success);
                        random::sampler_binomial_alias<uniform_type, bounds_type, type::diameter> sampler(distribution);
                        auto& alias = sampler.alias();
                        auto& cutoff = sampler.cutoff();

                        std::memcpy(&this->m_alias.unchecked_at(i, 0), alias.data(), alias.size() * sizeof(result_type)); // alias.size() = (n + 1)
                        std::memcpy(&this->m_cutoff.unchecked_at(i, 0), cutoff.data(), cutoff.size() * sizeof(double)); // cutoff.size() = (n + 1)
                    }
                }

            public:
                sampler_binomial_lookup()
                    : sampler_binomial_lookup(distribution_type(), distribution_type())
                {
                }

                type& operator =(const type& other)
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

                explicit sampler_binomial_lookup(const distribution_type& from, const distribution_type& to)
                    : m_number_of_trials_min(from.number_of_trials()), m_number_of_trials_max(to.number_of_trials()),
                    m_alias(this->height(), this->width()), m_cutoff(this->height(), this->width())
                {
                    if (from.probability_of_success() != to.probability_of_success()) throw std::out_of_range("<from> and <to> must have the same probability of success.");
                    if (this->m_number_of_trials_min < 1) throw std::out_of_range("Number of trials in <from> must be at least one.");
                    if (this->height() < 1) throw std::out_of_range("Number of trials in <to> must not be less than that in <from>.");

                    // No need to initialize matrices, since they will be filled by build(...).
                    this->build(from.probability_of_success());
                }

                std::size_t number_of_trials_min() const { return this->m_number_of_trials_min; }

                std::size_t number_of_trials_max() const { return this->m_number_of_trials_max; }

                std::size_t height() const
                {
                    return this->m_number_of_trials_max - this->m_number_of_trials_min + 1UL;
                }

                std::size_t width() const
                {
                    return this->m_number_of_trials_max + 1UL;
                }

                std::size_t size_in_bytes() const
                {
                    std::size_t k = this->height() * this->width();
                    return k * (sizeof(result_type) + sizeof(double));
                }
                
                //// Set the number of trials used in the generator; should be between <t_number_of_trials_min> and <t_number_of_trials_max>.
                //void at(result_type number_of_trials)
                //{
                //    if (number_of_trials < this->m_number_of_trials_min || number_of_trials > this->m_number_of_trials_max)
                //        throw std::out_of_range("<number_of_trials> should be between <number_of_trials_min> and <number_of_trials_max>.");
                //    this->m_current_row = number_of_trials - this->m_number_of_trials_min;
                //}

                template <typename t_engine_type>
                result_type operator ()(result_type number_of_trials, t_engine_type& uniform_generator) const
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to be equal to <diameter>.");
                    
                    result_type current_row = number_of_trials - this->m_number_of_trials_min;
                    double uniform_random = (uniform_generator() - t_engine_type::min()) / static_cast<double>(type::diameter + 1);

                    double u = (number_of_trials + 1) * uniform_random; // uniform continuous \in[0, n + 1).
                    result_type index = static_cast<result_type>(u);    // uniform discrete   \in[0, n].
                    u = (index + 1) - u;                                // 1 - overshoot: uniform continuous \in(0, 1].
                    return (u > this->m_cutoff.unchecked_at(current_row, index)) ? this->m_alias.unchecked_at(current_row, index) : index;
                }
            };

            template <typename t_engine_type>
            using default_sampler_binomial_lookup_t = sampler_binomial_lookup<typename t_engine_type::result_type, std::size_t, t_engine_type::max() - t_engine_type::min()>;
        }
    }
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_SAMPLER_BINOMIAL_LOOKUP_HPP_INCLUDED
