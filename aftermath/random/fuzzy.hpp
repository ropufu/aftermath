
#ifndef ROPUFU_AFTERMATH_RANDOM_FUZZY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_FUZZY_HPP_INCLUDED

#include "../not_an_error.hpp"

#include <cmath>   // std::nan, std::isnan
#include <cstddef> // std::size_t
#include <map>     // std::map
#include <string>  // std::string, std::to_string

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            /** Class for handing functions whose values are not known but evaluated
             *  empirically (e.g., via simulations). If \c f is the unknown function,
             *  then one observes <tt>f(x) + e</tt>, where \c e are random errors
             *  with mean zero.
             */
            struct fuzzy
            {
                using type = fuzzy;
                using local_coordinate_type = std::int_fast64_t;
                using global_coordinate_type = double;
                using value_type = double;

            private:
                //    -2     -1      0      1         local coordinates 
                //  ---.------.------x------.------.---------->         
                //            |    anchor   |      |                    
                //            |<---->|<---->|<---->|                    
                //            | unit | unit | unit |                    
                global_coordinate_type m_grid_anchor = std::nan(""); // Where the grid is centered.
                global_coordinate_type m_grid_unit = std::nan("");   // Unit step of the grid.
                std::map<local_coordinate_type, value_type> m_observations = { }; // Observed key-value pairs. Keys correspond to local grid coordinates.

                local_coordinate_type m_local_position = 0;
                local_coordinate_type m_local_step = 0;

                bool m_is_bad = false; // Indicates if the fuzzy descriptor has encountered a problem.
                global_coordinate_type m_lower_bound = std::nan(""); // Empirical lower bound on the zero of the function.
                global_coordinate_type m_upper_bound = std::nan(""); // Empirical upper bound on the zero of the function.

                /** Private unchecked constructor. */
                explicit fuzzy(global_coordinate_type initial_argument, global_coordinate_type initial_step, std::size_t tail_size, std::size_t repeat, std::size_t max_steps) noexcept
                {
                    if (std::isnan(initial_argument)) this->on_error("<initial_argument> cannot be NaN.");
                    if (std::isnan(initial_step))     this->on_error("<initial_step> cannot be NaN.");
                    if (initial_step == 0) this->on_error("<initial_step> cannot be zero.");
                    if (tail_size == 0)    this->on_error("<tail_size> cannot be zero.");
                    if (repeat == 0)       this->on_error("<repeat> cannot be zero.");
                    if (max_steps == 0)    this->on_error("<max_steps> cannot be zero.");

                    if (this->m_is_bad) return;
                    
                    // Make sure initial step is positive.
                    if (initial_step < 0) initial_step = -initial_step;
                    // Initialize grid.
                    local_coordinate_type factor = 1;
                    for (std::size_t i = 0; i < repeat; ++i)
                    {
                        local_coordinate_type next_factor = (factor << 1); // Double the grid spacing.
                        if (next_factor <= factor) { this->on_error("<repeat> overflow."); return; }
                        factor = next_factor;
                    }
                    initial_step = static_cast<global_coordinate_type>(initial_step / factor);
                    if (initial_step == 0) { this->on_error("<repeat> exceeds machine accuracy."); return; }

                    this->m_grid_anchor = initial_argument;
                    this->m_grid_unit = initial_step;
                    this->m_local_position = 0;
                    this->m_local_step = factor;
                } // fuzzy(...)

                /** Translates local coordinate to global. */
                global_coordinate_type local_to_global(local_coordinate_type local_coordinate) const noexcept { return static_cast<global_coordinate_type>(this->m_grid_anchor + local_coordinate * this->m_grid_unit); }

                /** Translates global coordinate to local. Do not use for indexing! */
                local_coordinate_type global_to_local(global_coordinate_type global_coordinate) const noexcept { return static_cast<local_coordinate_type>((global_coordinate - this->m_grid_anchor) / this->m_grid_unit); }

                template <typename t_function_type>
                value_type eval_local(t_function_type& f, local_coordinate_type local_argument) noexcept
                {
                    const auto& lower_bound_iterator = this->m_observations.lower_bound(local_argument); // Points to the first element that is not less than <argument>.
                    
                    bool is_known = (lower_bound_iterator != this->m_observations.end());
                    if (is_known && ((lower_bound_iterator->first) == local_argument)) return lower_bound_iterator->second;

                    global_coordinate_type global_argument = this->local_to_global(local_argument);
                    value_type value = f(global_argument);
                    this->m_observations.emplace_hint(lower_bound_iterator, local_argument, value);
                    if (std::isnan(value)) this->on_error("Evaluation failed at argument " + std::to_string(global_argument) + ".");
                    return value;
                } // eval_local(...)

                global_coordinate_type on_error(const std::string& message) noexcept
                {
                    this->m_is_bad = true;
                    aftermath::quiet_error::instance().push(aftermath::not_an_error::runtime_error, aftermath::severity_level::major, message, __FUNCTION__, __LINE__);
                    return std::nan("");
                } // on_error(...)

                global_coordinate_type on_error(std::string&& message) noexcept
                {
                    this->m_is_bad = true;
                    aftermath::quiet_error::instance().push(aftermath::not_an_error::runtime_error, aftermath::severity_level::major, message, __FUNCTION__, __LINE__);
                    return std::nan("");
                } // on_error(...)

            public:
                bool bad() const noexcept { return this->m_is_bad; }
                bool good() const noexcept { return !(this->m_is_bad); }

                global_coordinate_type lower_bound() const noexcept { return this->m_lower_bound; }
                global_coordinate_type upper_bound() const noexcept { return this->m_upper_bound; }

                global_coordinate_type estimate(global_coordinate_type relative_position = 0.5) const noexcept { return ((1 - relative_position) * this->m_lower_bound + relative_position * this->m_upper_bound); }

                template <typename t_function_type>
                static type find_zero_increasing(t_function_type& f, global_coordinate_type initial_argument, global_coordinate_type initial_step, std::size_t tail_size, std::size_t repeat, std::size_t max_steps) noexcept
                {
                    type result(initial_argument, initial_step, tail_size, repeat, max_steps);
                    if (result.m_is_bad) return result;

                    result.m_lower_bound = result.zero_bound(f, true, true, tail_size, repeat, max_steps);
                    result.m_upper_bound = result.zero_bound(f, true, false, tail_size, repeat, max_steps);
                    return result;
                } // find_zero_increasing(...)

                template <typename t_function_type>
                static type find_zero_decreasing(t_function_type& f, global_coordinate_type initial_argument, global_coordinate_type initial_step, std::size_t tail_size, std::size_t repeat, std::size_t max_steps) noexcept
                {
                    type result(initial_argument, initial_step, tail_size, repeat, max_steps);
                    if (result.m_is_bad) return result;

                    result.m_lower_bound = result.zero_bound(f, false, true, tail_size, repeat, max_steps);
                    result.m_upper_bound = result.zero_bound(f, false, false, tail_size, repeat, max_steps);
                    return result;
                } // find_zero_decreasing(...)

            private:
                /** @brief Tries to find a zero of a noisy function.
                 *  @return Fuzzy descriptor of what has been done.
                 */
                template <typename t_function_type>
                global_coordinate_type zero_bound(t_function_type& f, bool is_increasing, bool is_positive_direction,
                    std::size_t tail_size, std::size_t repeat, std::size_t max_steps) noexcept
                {
                    std::size_t step_count = 0;

                    // ====================================
                    // f      | increasing  | decreasing  |
                    // h > 0  | ----(+)     | ++++(-)     |
                    // h < 0  | (-)++++     | (+)----     |
                    // ====================================
                    value_type sign = (is_increasing ^ is_positive_direction) ? -1 : 1;

                    local_coordinate_type x = this->m_local_position;
                    local_coordinate_type step = is_positive_direction ? this->m_local_step : (-this->m_local_step);

                    // First step: find the argument where the function has correct <sign>.
                    value_type y = this->eval_local(f, x);
                    if (this->m_is_bad) return std::nan("");

                    while (sign * y < 0)
                    {
                        // Take a step.
                        x += step; // @todo Think about overflow handling.
                        // Evaluate the function.
                        y = this->eval_local(f, x);
                        if (this->m_is_bad) return std::nan("");
                        
                        ++step_count;
                        if (step_count == max_steps) return this->on_error("Maximum number of steps reached.");
                    }

                    // Second step: start at the correct position, and push it as far as possible.
                    local_coordinate_type b = x;
                    for (std::size_t i = 0; i < repeat; ++i)
                    {
                        step_count = 0; // Reset step counter.
                        step /= 2; // Refine grid.

                        std::size_t count_negatives = 0;
                        while (count_negatives < tail_size)
                        {
                            x = b;
                            for (std::size_t j = 0; j < tail_size; ++j)
                            {
                                // Take a step.
                                x -= step; // @todo Think about overflow handling.
                                // Evaluate the function.
                                y = this->eval_local(f, x);
                                if (this->m_is_bad) return std::nan("");
                                // Check the sign.
                                if (sign * y < 0) ++count_negatives;
                                else
                                {
                                    count_negatives = 0;
                                    b = x;
                                    break;
                                }
                            }

                            ++step_count;
                            if (step_count == max_steps) return this->on_error("Maximum number of steps reached.");
                        }
                    }
                    return (b - step);
                } // zero_bound(...)
            }; // struct fuzzy
        } // namespace random
    } // namespace aftermath
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_RANDOM_FUZZY_HPP_INCLUDED
