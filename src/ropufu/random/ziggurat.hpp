
#ifndef ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED

#include <cstddef> // std:size_t
#include <type_traits> // std::is_same_v

namespace ropufu::aftermath::random
{
    /** @brief Base class for ziggurat pseudo-random number generators. */
    template <typename t_derived_type, typename t_engine_type, typename t_result_type, typename t_probability_type, std::size_t t_n_boxes>
    struct ziggurat
    {
        using type = ziggurat<t_derived_type, t_engine_type, t_result_type, t_probability_type, t_n_boxes>;
        using derived_type = t_derived_type;
        using engine_type = t_engine_type;
        using result_type = t_result_type;
        using probability_type = t_probability_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr std::size_t n_boxes = t_n_boxes;
        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();

    private:
        result_type sample_from_box_horizontal(uniform_type box_index, uniform_type uniform_random, bool& is_interior) const noexcept 
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::sample_from_box_horizontal), 
                decltype(&type::sample_from_box_horizontal)>;
            static_assert(is_overwritten, "static polymorphic function <sample_from_box_horizontal> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->sample_from_box_horizontal(box_index, uniform_random, is_interior);
        } // sample_from_box_horizontal(...)

        bool is_inside_box_vertical(uniform_type box_index, result_type horizontal, uniform_type uniform_random) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::is_inside_box_vertical), 
                decltype(&type::is_inside_box_vertical)>;
            static_assert(is_overwritten, "static polymorphic function <is_inside_box_vertical> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->is_inside_box_vertical(box_index, horizontal, uniform_random);
        } // is_inside_box_vertical(...)

        bool is_tail_box(uniform_type box_index) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::is_tail_box), 
                decltype(&type::is_tail_box)>;
            static_assert(is_overwritten, "static polymorphic function <is_tail_box> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->is_tail_box(box_index);
        } // is_tail_box(...)

        result_type sample_tail(uniform_type box_index, engine_type& uniform_generator) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::sample_tail), 
                decltype(&type::sample_tail)>;
            static_assert(is_overwritten, "static polymorphic function <sample_tail> was not overwritten.");
            derived_type* that = static_cast<derived_type*>(this);
            return that->sample_tail(box_index, uniform_generator);
        } // sample_tail(...)

    public:
        result_type sample(engine_type& uniform_generator) noexcept
        {
            static_assert(aftermath::is_power_of_two(n_boxes), "Number of boxes has to be a power of two.");
            constexpr uniform_type box_mask = static_cast<uniform_type>(n_boxes - 1);
            while (true)
            {
                uniform_type u1 = uniform_generator() - engine_type::min(); // Used for box index.
                uniform_type u2 = uniform_generator() - engine_type::min(); // Used to generate the horizontal component.

                // ~~ Pick a box ~~
                uniform_type box_index = static_cast<uniform_type>(u1 & box_mask);

                // ~~ Optimized rejection ~~
                bool is_interior = false;
                result_type z = this->sample_from_box_horizontal(box_index, u2, is_interior);
                if (is_interior) return z;

                // ~~ Tails ~~
                if (this->is_tail_box(box_index)) return this->sample_tail(box_index, uniform_generator);

                // ~~ Standard rejection ~~
                uniform_type u3 = uniform_generator() - engine_type::min(); // Used to generate the vertical component.
                if (this->is_inside_box_vertical(box_index, z, u3)) return z;
            } // while(...)
        } // sample(...)
    }; // struct ziggurat
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_HPP_INCLUDED
