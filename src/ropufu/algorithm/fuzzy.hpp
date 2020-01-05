
#ifndef ROPUFU_AFTERMATH_ALGORITHM_FUZZY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_FUZZY_HPP_INCLUDED

#include "../number_traits.hpp"

#include <cstddef>      // std::size_t
#include <cstdint>      // std::int_fast64_t
#include <functional>   // std::function
#include <limits>       // std::numeric_limits
#include <map>          // std::map
#include <stdexcept>    // std::logic_error, std::runtime_error
#include <string>       // std::string, std::to_string
#include <system_error> // std::error_code, std::errc, std::make_error_code
#include <type_traits>  // std::is_floating_point_v
#include <utility>      // std::move

namespace ropufu::aftermath::algorithm
{
    /** Class for handing functions whose values are not known but evaluated
     *  empirically (e.g., via simulations). If \c f is the unknown function,
     *  then one observes f + e, where \c e are random errors with mean zero.
     */
    template <typename t_argument_type, typename t_value_type>
    struct fuzzy
    {
        using type = fuzzy<t_argument_type, t_value_type>;
        using argument_type = t_argument_type;
        using value_type = t_value_type;

        using function_type = std::function<value_type (argument_type)>;
        using local_coordinate_type = std::int_fast64_t;
        using global_coordinate_type = argument_type;

        static constexpr local_coordinate_type bound_margin = 8; // How far along the same-sign streak you want to move.
        static constexpr local_coordinate_type default_grid_resolution = 32;
        static constexpr local_coordinate_type default_tail_length = 2;
        static constexpr std::size_t default_step_limit = 5'000;

    private:
        function_type m_noisy_function;
        // Grid:
        //    -2     -1      0      1         local coordinates  
        //  ---.------.------x------.------.---------->          
        //            |    anchor   |      |  global coordinates 
        //            |<---->|<---->|<---->|                     
        //            | unit | unit | unit |                      
        global_coordinate_type m_grid_anchor = std::numeric_limits<global_coordinate_type>::quiet_NaN(); // Where the grid is centered.
        global_coordinate_type m_grid_unit = std::numeric_limits<global_coordinate_type>::quiet_NaN();   // Unit step of the grid.
        local_coordinate_type m_grid_resolution = type::default_grid_resolution;
        // Options.
        local_coordinate_type m_tail_length = type::default_tail_length;
        std::size_t m_max_steps = type::default_step_limit;
        // Cached values.
        std::map<local_coordinate_type, value_type> m_observations = {}; // Observed key-value pairs. Keys correspond to local grid coordinates.

        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_floating_point_v<argument_type>, "Function argument has to be a floating point type.");
            static_assert(std::is_floating_point_v<value_type>, "Function value has to be a floating point type.");
        } // traits_check(...)

        /** Translates local coordinates to global. */
        global_coordinate_type local_to_global(local_coordinate_type local_coordinate) const noexcept
        {
            return static_cast<global_coordinate_type>(this->m_grid_anchor + local_coordinate * this->m_grid_unit);
        } // local_to_global(...)

        /** Translates global coordinates to local. */
        local_coordinate_type global_to_local(global_coordinate_type global_coordinate) const noexcept
        {
            return static_cast<local_coordinate_type>((global_coordinate - this->m_grid_anchor) / this->m_grid_unit);
        } // global_to_local(...)

        /** Evaluates the function at a grid point. If the function has already been evaluated, returns the cached value. */
        value_type eval_local(local_coordinate_type local_argument)
        {
            const auto& lower_bound_iterator = this->m_observations.lower_bound(local_argument); // Points to the first element that is not less than local_argument.
            
            bool is_known = (lower_bound_iterator != this->m_observations.end());
            if (is_known && ((lower_bound_iterator->first) == local_argument)) return lower_bound_iterator->second;

            global_coordinate_type global_argument = this->local_to_global(local_argument);
            value_type value = this->m_noisy_function(global_argument);
            this->m_observations.emplace_hint(lower_bound_iterator, local_argument, value);
            
            if (aftermath::is_nan(value)) throw std::runtime_error("Evaluation failed at argument " + std::to_string(global_argument) + ".");
            return value;
        } // eval_local(...)

        /** @brief Tries to find a zero of a noisy function. */
        template <bool t_is_increasing, bool t_is_positive_direction>
        local_coordinate_type zero_bound(std::error_code& ec)
        {
            std::size_t step_count = 0;

            // ====================================
            // f      | increasing  | decreasing  |
            // h > 0  | ----(+)     | ++++(-)     |
            // h < 0  | (-)++++     | (+)----     |
            // ====================================
            constexpr value_type sign = (t_is_increasing ^ t_is_positive_direction) ? -1 : 1;

            local_coordinate_type x = 0;
            local_coordinate_type step = t_is_positive_direction ? this->m_grid_resolution : (-this->m_grid_resolution);
            local_coordinate_type step_abs = this->m_grid_resolution;

            // First step: find the argument where the function has correct sign.
            value_type y = this->eval_local(x);
            while (sign * y < 0)
            {
                // Take a step.
                x += step; // @todo Think about overflow handling.
                y = this->eval_local(x);
                
                ++step_count;
                if (step_count == this->m_max_steps) // Maximum number of steps reached.
                {
                    ec = std::make_error_code(std::errc::operation_canceled);
                    return 0;
                } // if (...)
            } // while (...)

            // Second step: start at the correct position, and push it as far as possible.
            local_coordinate_type b = x;
            while (true)
            {
                step_count = 0; // Reset step counter.
                // Refine grid.
                if (step / 2 != 0)
                {
                    step /= 2;
                    step_abs /= 2;
                } // if (...)
                // Keep tail count in sync with grid refinement.
                local_coordinate_type tail_count_required = (this->m_tail_length * this->m_grid_resolution) / step_abs;

                local_coordinate_type count_negatives = 0;
                while (count_negatives < tail_count_required)
                {
                    x = b;
                    for (local_coordinate_type j = 0; j < tail_count_required; ++j)
                    {
                        // Take a step.
                        x -= step; // @todo Think about overflow handling.
                        y = this->eval_local(x);
                        // Check the sign.
                        if (sign * y < 0) ++count_negatives;
                        else // Shift the bound: the sign is wrong.
                        {
                            count_negatives = 0;
                            b = x;
                            break; // for (...)
                        } // if (...)
                    } // for (...)

                    ++step_count;
                    if (step_count == this->m_max_steps) // Maximum number of steps reached.
                    {
                        ec = std::make_error_code(std::errc::operation_canceled);
                        return 0;
                    } // if (...)
                } // while (...)

                if (step_abs == 1) break; // while (...)
            } // while (...)
            return (b - type::bound_margin * step);
        } // zero_bound(...)

    public:
        /*implicit*/ fuzzy(function_type&& noisy_function) noexcept
            : m_noisy_function(std::move(noisy_function))
        {
            type::traits_check();
        } // fuzzy(...)

        /** @brief Initialize the grid for searching for zero.
         *  @param initial_argument The origin of the grid.
         *  @param initial_step The initial step to take when searching the grid.
         *  @param grid_resolution Number of sub-intervals in an interval of length \p initial_step.
         */
        void initialize_grid(argument_type initial_argument, argument_type initial_step, local_coordinate_type grid_resolution = type::default_grid_resolution)
        {
            if (!aftermath::is_finite(initial_argument)) throw std::logic_error("Initial argument must be finite.");
            if (!aftermath::is_finite(initial_step)) throw std::logic_error("Initial step must be finite.");
            if (initial_step == 0) throw std::logic_error("Initial step must be non-zero.");
            if (grid_resolution < 1) throw std::logic_error("Grid resolution must be at least 1.");

            // Initialize grid.
            global_coordinate_type grid_unit = (initial_step < 0) ? (-initial_step) : (initial_step);
            grid_unit /= grid_resolution;
            if (grid_unit == 0) throw std::runtime_error("Grid is too fine to represent.");

            this->m_grid_anchor = initial_argument;
            this->m_grid_unit = grid_unit;
            this->m_grid_resolution = grid_resolution;

            this->m_observations.clear();
        } // initialize(...)

        /** @brief Sets options for searching for zero.
         *  @param tail_length The length of the interval, expressed in units of initial step,
         *    where the sign of the function has to stay the same.
         *  @param max_steps Cap on the maximum number of steps to be taken while searching
         *    for zero. If the cap is exceeded, the error code in the corresponding function
         *    call will be set to \c std::errc::operation_canceled.
         */
        void options(std::size_t tail_length, std::size_t max_steps = type::default_step_limit)
        {
            if (tail_length < 1) throw std::logic_error("Tail length must be at least 1.");
            if (tail_length * this->m_grid_resolution < type::bound_margin) throw std::logic_error("Tail must contain at least 8 grid units.");
            if (max_steps == 0) throw std::logic_error("Maximum number of steps must be at least 1.");

            this->m_tail_length = tail_length;
            this->m_max_steps = max_steps;
        } // options(...)

        /** Tries to find the zero of the function, assuming it is strictly increasing. */
        void find_zero_increasing(argument_type& lower_bound, argument_type& upper_bound, std::error_code& ec)
        {
            local_coordinate_type local_lower_bound = this->template zero_bound<true, true>(ec);
            local_coordinate_type local_upper_bound = this->template zero_bound<true, false>(ec);

            if (ec.value() != 0) return;
            lower_bound = this->local_to_global(local_lower_bound);
            upper_bound = this->local_to_global(local_upper_bound);
        } // find_zero_increasing(...)

        /** Tries to find the zero of the function, assuming it is strictly decreasing. */
        void find_zero_decreasing(argument_type& lower_bound, argument_type& upper_bound, std::error_code& ec)
        {
            local_coordinate_type local_lower_bound = this->template zero_bound<false, true>(ec);
            local_coordinate_type local_upper_bound = this->template zero_bound<false, false>(ec);

            if (ec.value() != 0) return;
            lower_bound = this->local_to_global(local_lower_bound);
            upper_bound = this->local_to_global(local_upper_bound);
        } // find_zero_decreasing(...)
    }; // struct fuzzy
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_FUZZY_HPP_INCLUDED
