
#ifndef ROPUFU_P0870_HPP_INCLUDED
#define ROPUFU_P0870_HPP_INCLUDED

// Taken from http://open-std.org/JTC1/SC22/WG21/docs/papers/2020/p0870r3.html

#include <concepts>    // std::same_as
#include <type_traits> // std::type_identity_t, std::is_convertible_v
#include <utility>     // std::forward

namespace ropufu
{
    namespace detail
    {
        template <typename t_from_type, typename t_to_type>
        concept construct_without_narrowing = requires(t_from_type&& x)
        {
            {std::type_identity_t<t_to_type[]>{std::forward<t_from_type>(x)}} -> std::same_as<t_to_type[1]>;
        }; // concept construct_without_narrowing
    } // namespace detail

    template <typename t_from_type, typename t_to_type>
    inline constexpr bool is_narrowing_convertible_v = false;

    template <typename t_from_type, typename t_to_type>
        requires std::is_convertible_v<t_from_type, t_to_type>
    inline constexpr bool is_narrowing_convertible_v<t_from_type, t_to_type> =
        !detail::construct_without_narrowing<t_from_type, t_to_type>;
} // namespace ropufu

#endif // ROPUFU_P0870_HPP_INCLUDED
