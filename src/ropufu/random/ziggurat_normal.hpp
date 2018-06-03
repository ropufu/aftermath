
#ifndef ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_NORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_NORMAL_HPP_INCLUDED

#include "../probability/normal_distribution.hpp"
#include "boxes_normal_1024.hpp"
#include "ziggurat.hpp"

#include <cmath>   // std::sqrt, std::log
#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    /** @brief Ziggurat for normal distribution. */
    template <typename t_engine_type, typename t_result_type, std::size_t t_n_boxes>
    struct ziggurat_normal
        : public ziggurat<ziggurat_normal<t_engine_type, t_result_type, t_n_boxes>, t_engine_type, t_result_type, t_n_boxes>,
        public detail::boxes_normal<t_engine_type, t_result_type, t_n_boxes>
    {
        using type = ziggurat_normal<t_engine_type, t_result_type, t_n_boxes>;
        using ziggurat_type = ziggurat<type, t_engine_type, t_result_type, t_n_boxes>;
        using box_type = detail::boxes_normal<t_engine_type, t_result_type, t_n_boxes>;

        using engine_type = t_engine_type;
        using distribution_type = probability::normal_distribution<t_result_type>;
        using result_type = typename distribution_type::result_type;
        using param_type = typename distribution_type::param_type;
        using uniform_type = typename t_engine_type::result_type;

        static constexpr std::size_t n_boxes = t_n_boxes;
        static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
        // Zero-based index of the box that covers the right tail.
        static constexpr std::size_t right_tail_index = 0;
        // Zero-based index of the box that covers the left tail.
        static constexpr std::size_t left_tail_index = n_boxes - 1;

        friend ziggurat_type;

    private:
        probability::normal_distribution<result_type> m_distribution = probability::normal_distribution<result_type>::standard;
        result_type pdf(result_type x) const noexcept { return this->m_distribution.pdf(x); }
        
        result_type sample_from_box_horizontal(uniform_type box_index, uniform_type uniform_random, bool& is_interior) const noexcept
        {
            result_type z = uniform_random * static_cast<result_type>(this->m_width_scaled[box_index]);
            is_interior = (uniform_random < this->m_coverage_scaled[box_index]);
            return z;
        } // sample_from_box_horizontal(...)

        bool is_inside_box_vertical(uniform_type box_index, result_type horizontal, uniform_type uniform_random) const noexcept
        {
            result_type f = this->pdf(horizontal);
            return (static_cast<result_type>(this->m_bottom[box_index]) + uniform_random * static_cast<result_type>(this->m_height_scaled[box_index]) < f);
        } // is_inside_box_vertical(...)

        bool is_tail_box(uniform_type box_index) const noexcept
        {
            switch (box_index)
            {
            case 0: return true;
            case n_boxes - 1: return true;
            default: return false;
            }
        } // is_tail_box(...)

        result_type sample_tail(uniform_type box_index, engine_type& uniform_generator) noexcept
        {
            const result_type r = box_type::right_tail_x;
            const result_type r_squared = r * r;
            result_type result = 0;
            result_type u1, u2;
            while (true)
            {
                u1 = (uniform_generator() - engine_type::min()) / box_type::modulus;
                u2 = (uniform_generator() - engine_type::min()) / box_type::modulus;
                result = static_cast<result_type>(std::sqrt(r_squared - 2 * std::log(1 - u1)));
                if (u2 * result < r) return box_index == 0 ? result : -result;
            }
        } // sample_tail(...)
    }; // struct ziggurat_normal
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_NORMAL_HPP_INCLUDED
