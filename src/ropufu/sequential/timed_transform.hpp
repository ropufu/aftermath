
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_TIMED_TRANSFORM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_TIMED_TRANSFORM_HPP_INCLUDED

#include "../simple_vector.hpp"

#include <concepts>  // std::same_as, std::totally_ordered
#include <cstddef>   // std::size_t
#include <ranges>    // std::ranges::...
#include <stdexcept> // std::logic_error
#include <utility>   // std::declval

namespace ropufu::aftermath::sequential
{
    /** A transformation f that behaves like a function of two arguments,
     *  f(std::size_t time, t_value_type x) const -> t_value_type.
     *  Aimed at facilitation transition period for window-limited statistics.
     *  E.g., if the detection statistic R_n = sum (last L observations), the
     *  transition period would be R_1, R_2, ..., R_{L - 1}.
     */
    template <typename t_transform_type, typename t_value_type>
    concept timed_transform = requires(const t_transform_type& f, std::size_t time, t_value_type value)
    {
        {f(time, value)} -> std::same_as<t_value_type>;
    }; // concept timed_transform

    /** Identity transformation. */
    template <typename t_value_type>
    struct identity_transform
    {
        using type = identity_transform<t_value_type>;
        using value_type = t_value_type;

        constexpr value_type operator ()(std::size_t time, value_type x) const noexcept
        {
            return x;
        } // operator ()(...)
    }; // struct identity_transform

    /** Window-limited linear transformation. */
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<t_value_type>>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct window_limited_linear_transform
    {
        using type = window_limited_linear_transform<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

    private:
        container_type m_scale_factors = {};
        container_type m_shifts = {};
        std::size_t m_window_size = 0;

    public:
        window_limited_linear_transform() noexcept = default;

        window_limited_linear_transform(container_type scale_factors, container_type shifts)
            : m_scale_factors(scale_factors), m_shifts(shifts)
        {
            if (this->m_scale_factors.size() != this->m_shifts.size()) throw std::logic_error("Scale factors and shifts should contain the same number of elements.");
            this->m_window_size = this->m_scale_factors.size();
        } // window_limited_linear_transform(...)

        constexpr value_type operator ()(std::size_t time, value_type x) const noexcept
        {
            if (time >= this->m_window_size) return x;
            return this->m_scale_factors[time] * x + this->m_shifts[time];
        } // operator ()(...)
    }; // struct linear_transform
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_TIMED_TRANSFORM_HPP_INCLUDED
