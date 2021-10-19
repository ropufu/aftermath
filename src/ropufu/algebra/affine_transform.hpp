
#ifndef ROPUFU_AFTERMATH_ALGEBRA_AFFINE_TRANSFORM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_AFFINE_TRANSFORM_HPP_INCLUDED

#include <concepts> // std::totally_ordered
#include <ranges>   // std::ranges::...

namespace ropufu::aftermath::algebra
{
    /** Affine transformation in R^1.
     *  @todo Add specialization for R^2.
     */
    template <std::totally_ordered t_value_type>
    struct affine_transform
    {
        using type = affine_transform<t_value_type>;
        using value_type = t_value_type;

        value_type scale;
        value_type shift;

        /** Transform a single value. */
        value_type operator ()(value_type value) const noexcept
        {
            value *= this->scale;
            value += this->shift;
            return value;
        } // operator ()(...)

        /** Transform a block of values. */
        template <std::ranges::range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        void operator ()(t_container_type& values) noexcept
        {
            for (value_type& x : values)
            {
                x *= this->scale;
                x += this->shift;
            } // for (...)
        } // next(...)
    }; // struct affine_transform
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_AFFINE_TRANSFORM_HPP_INCLUDED
