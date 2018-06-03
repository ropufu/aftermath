
#ifndef ROPUFU_AFTERMATH_RANDOM_BOXES_NORMAL_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_BOXES_NORMAL_HPP_INCLUDED

#include <cstddef> // std::size_t

namespace ropufu::aftermath::random
{
    namespace detail
    {
        template <typename t_engine_type, typename t_result_type, std::size_t t_n_boxes>
        struct boxes_normal
        {
            using type = boxes_normal<t_engine_type, t_result_type, t_n_boxes>;
            using engine_type = t_engine_type;
            using uniform_type = typename t_engine_type::result_type;
            using result_type = t_result_type;

            static constexpr std::size_t n_boxes = t_n_boxes;
            static constexpr uniform_type diameter = engine_type::max() - engine_type::min();
            static constexpr result_type modulus = static_cast<result_type>(diameter) + 1;

            static constexpr result_type right_tail_x = 0;
            static constexpr result_type left_tail_x = 0;
            static constexpr result_type box_volume = 0;
            static constexpr uniform_type box_volume_diameter = static_cast<uniform_type>(box_volume * modulus);

        protected:
            const result_type m_width_scaled[n_boxes]; // (<diameter> + 1)-downscaled right (left) x-endpoints of the boxes, modified for the tail boxes: { x_0*, x_1, x_2, ..., x_{n - 1}, x_n* }.
            const result_type m_height_scaled[n_boxes]; // (<diameter> + 1)-downscaled heights of the boxes: { f(x_1), f(x_2) - f(x_1), ..., f(x_{n - 2}) - f(x_{n - 1}), f(x_{n - 2}) }.
            const result_type m_bottom[n_boxes]; // Density at the proper x-endpoints of the boxes, tail boxes treated specially: { 0, f(x_1), f(x_2), ..., f(x_{n - 2}), 0 }.
            const uniform_type m_coverage_scaled[n_boxes]; // (<diameter> + 1)-upscaled probability of simple coverage of the boxes: { (x_1 - m) / (x_0* - m), (x_2 - m) / (x_1 - m), ..., (m - x_{n - 1}) / (m - x_n*) }.
        }; // struct boxes_normal
    } // namespace detail
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_BOXES_NORMAL_HPP_INCLUDED
