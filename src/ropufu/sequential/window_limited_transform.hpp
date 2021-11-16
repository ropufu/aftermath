
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_TRANSFORM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_TRANSFORM_HPP_INCLUDED

#include <concepts> // std::same_as, std::totally_ordered
#include <cstddef>  // std::size_t
#include <utility>  // std::declval

namespace ropufu::aftermath::sequential
{
    /** A transformation f that behaves like a function of two arguments,
     *  f(std::size_t time, value_type x) const -> value_type.
     */
    template <typename t_type>
    concept window_limited_transform = requires(const t_type& f, std::size_t time)
    {
        typename t_type::value_type;

        {f(time, std::declval<typename t_type::value_type>())} -> std::same_as<typename t_type::value_type>;
    }; // concept distribution

    /** A helper class to handle transition period in window-limited statistics.
     *  E.g., if the detection statistic R_n = sum (last L observations), the
     *  transition period would be R_1, R_2, ..., R_{L - 1}.
     */
    template <std::totally_ordered t_value_type>
    struct identity_transform
    {
        using type = window_limited_statistic<t_value_type>;
        using value_type = t_value_type;

        constexpr value_type operator ()(std::size_t time, value_type x) const noexcept
        {
            return x;
        } // operator ()(...)
    }; // struct identity_transform
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_TRANSFORM_HPP_INCLUDED
