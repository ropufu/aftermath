
#ifndef ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED

#include "../template_math.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace random
        {
            namespace detail
            {
                template <std::size_t t_n_boxes, typename t_uniform_type>
                struct ziggurat_choose_box
                {
                    using uniform_type = t_uniform_type;

                    static std::uint_fast32_t choose_box(uniform_type uniform_random) noexcept;
                };

                template <std::size_t t_n_boxes>
                struct ziggurat_choose_box<t_n_boxes, std::uint_fast32_t>
                {
                    using uniform_type = std::uint_fast32_t;

                    static std::uint_fast32_t choose_box(uniform_type uniform_random) noexcept
                    {
                        static_assert(is_power_of_two<t_n_boxes>::value, "<t_n_boxes> has to be a power of two.");
                        return uniform_random & (t_n_boxes - 1);
                    }
                };
            }

            /** @brief Ziggurat pseudo-random number generator, intended as CRTP for static polymorhism.
             *  @remark For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
             */
            template <typename t_derived_type, std::size_t t_n_boxes, typename t_uniform_type, typename t_bounds_type, t_bounds_type t_diameter, typename t_result_type>
            struct ziggurat
            {
                using type = ziggurat<t_derived_type, t_n_boxes, t_uniform_type, t_bounds_type, t_diameter, t_result_type>;
                static constexpr std::size_t n_boxes = t_n_boxes;
                static constexpr t_bounds_type diameter = t_diameter;

                using derived_type = t_derived_type;
                using uniform_type = t_uniform_type;
                using result_type = t_result_type;
                using bounds_type = t_bounds_type;

            private:
                result_type sample_from_box_horizontal(uniform_type box_index, uniform_type uniform_random, bool& is_interior) const noexcept 
                {
                    constexpr bool is_overwritten = std::is_same<
                        decltype(&derived_type::sample_from_box_horizontal), 
                        decltype(&type::sample_from_box_horizontal)>::value;
                    static_assert(!is_overwritten, "static polymorphic function <sample_from_box_horizontal> was not overwritten.");
                    const derived_type* that = static_cast<const derived_type*>(this);
                    return that->sample_from_box_horizontal(box_index, uniform_random, is_interior);
                }

                bool is_inside_box_vertical(uniform_type box_index, result_type horizontal, uniform_type uniform_random) const noexcept
                {
                    constexpr bool is_overwritten = std::is_same<
                        decltype(&derived_type::is_inside_box_vertical), 
                        decltype(&type::is_inside_box_vertical)>::value;
                    static_assert(!is_overwritten, "static polymorphic function <is_inside_box_vertical> was not overwritten.");
                    const derived_type* that = static_cast<const derived_type*>(this);
                    return that->is_inside_box_vertical(box_index, horizontal, uniform_random);
                }

                bool is_tail_box(uniform_type box_index) const noexcept
                {
                    constexpr bool is_overwritten = std::is_same<
                        decltype(&derived_type::is_tail_box), 
                        decltype(&type::is_tail_box)>::value;
                    static_assert(!is_overwritten, "static polymorphic function <is_tail_box> was not overwritten.");
                    const derived_type* that = static_cast<const derived_type*>(this);
                    return that->is_tail_box(box_index);
                }

                template <typename t_engine_type>
                result_type sample_tail(uniform_type box_index, t_engine_type& uniform_generator) noexcept
                {
                    constexpr bool is_overwritten = std::is_same<
                        decltype(&derived_type::template sample_tail<t_engine_type>), 
                        decltype(&type::template sample_tail<t_engine_type>)>::value;
                    static_assert(!is_overwritten, "static polymorphic function <sample_tail> was not overwritten.");
                    derived_type* that = static_cast<derived_type*>(this);
                    return that->sample_tail(box_index, uniform_generator);
                }

            public:
                template <typename t_engine_type>
                result_type sample(t_engine_type& uniform_generator) noexcept
                {
                    static_assert(std::is_same<typename t_engine_type::result_type, uniform_type>::value, "type mismatch");
                    static_assert(t_engine_type::max() - t_engine_type::min() == type::diameter, "<t_engine_type>::max() - <t_engine_type>::min() has to equal <diameter>.");

                    while (true)
                    {
                        uniform_type u1 = uniform_generator() - t_engine_type::min(); // Used for box index.
                        uniform_type u2 = uniform_generator() - t_engine_type::min(); // Used to generate the horizontal component.

                        // ~~ Pick a box ~~
                        std::uint_fast32_t box_index = detail::ziggurat_choose_box<n_boxes, uniform_type>::choose_box(u1);

                        // ~~ Optimized rejection ~~
                        bool is_interior = false;
                        result_type z = this->sample_from_box_horizontal(box_index, u2, is_interior);
                        if (is_interior) return z;

                        // ~~ Tails ~~
                        if (this->is_tail_box(box_index)) return this->sample_tail(box_index, uniform_generator);

                        // ~~ Standard rejection ~~
                        uniform_type u3 = uniform_generator() - t_engine_type::min(); // Used to generate the vertical component.
                        if (this->is_inside_box_vertical(box_index, z, u3)) return z;
                    }
                }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED
