
#ifndef ROPUFU_AFTERMATH_CONCEPTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_CONCEPTS_HPP_INCLUDED

#include <concepts>    // std::same_as, std::convertible_to, std::equality_comparable
#include <functional>  // std::hashable
#include <ostream>     // std::ostream
#include <ranges>      // std::ranges::range
#include <type_traits> // std::is_enum_v, std::is_arithmetic_v, std::decay_t
#include <utility>     // std::declval

namespace ropufu
{
    template <typename t_value_type>
    concept decayed = std::same_as<std::decay_t<t_value_type>, t_value_type>;

    template <typename t_container_type>
    concept push_back_container = std::ranges::range<t_container_type> &&
        requires(t_container_type& x)
        {
            typename t_container_type::value_type;
            {x.clear()};
            {x.push_back(std::declval<typename t_container_type::value_type>())};
        }; // concept push_back_container

    template <typename t_dictionary_type>
    concept emplace_dictionary = std::ranges::range<t_dictionary_type> &&
        requires(t_dictionary_type& x)
        {
            typename t_dictionary_type::key_type;
            typename t_dictionary_type::mapped_type;
            {x.clear()};
            {x.emplace(
                std::declval<typename t_dictionary_type::key_type>(),
                std::declval<typename t_dictionary_type::mapped_type>()).second} -> std::convertible_to<bool>;
        }; // concept emplace_dictionary

    template <typename t_dictionary_type, typename t_key_type>
    concept emplace_dictionary_with_key = emplace_dictionary<t_dictionary_type> &&
        std::same_as<typename t_dictionary_type::key_type, t_key_type>;

    /** @todo Keep an eye on concept support for parameter packs. */
    template <typename t_func_type>
    concept pure_action = requires(t_func_type& x)
    {
        {x()} -> std::same_as<void>;
    }; // concept pure_action

    template <typename t_func_type, typename t_arg_type>
    concept unary_action = requires(t_func_type& x, t_arg_type a)
    {
        {x(a)} -> std::same_as<void>;
    }; // concept unary_action

    template <typename t_func_type, typename t_first_arg_type, typename t_second_arg_type>
    concept binary_action = requires(t_func_type& x, t_first_arg_type a, t_second_arg_type b)
    {
        {x(a, b)} -> std::same_as<void>;
    }; // concept binary_action

    template <typename t_func_type, typename t_arg_type>
    concept unary_predicate = requires(t_func_type& x, t_arg_type a)
    {
        {x(a)} -> std::same_as<bool>;
    }; // concept unary_predicate

    template <typename t_func_type, typename t_first_arg_type, typename t_second_arg_type>
    concept binary_predicate = requires(t_func_type& x, t_first_arg_type a, t_second_arg_type b)
    {
        {x(a, b)} -> std::same_as<bool>;
    }; // concept binary_predicate

    template <typename t_container_type>
    concept wipeable = requires(t_container_type& x)
    {
        {x.wipe()} -> std::same_as<void>;
        {t_container_type::uninitialized(0)};
    }; // concept wipeable

    template <typename t_type>
    concept streamable = requires(std::ostream& os, const t_type& value)
    {
        {os << value} -> std::convertible_to<std::ostream&>;
    }; // concept streamable

    template <typename t_type>
    concept hashable = std::equality_comparable<t_type> && requires(const t_type& value)
    {
        {std::hash<t_type>()(value)};
    }; // concept hashable

    template <typename t_enum_type>
    concept enumeration = std::is_enum_v<t_enum_type>;

    template <typename t_numeric_type>
    concept numeric_signed = std::numeric_limits<t_numeric_type>::is_signed;

    template <typename t_numeric_type>
    concept integer = std::numeric_limits<t_numeric_type>::is_integer;

    template <typename t_numeric_type>
    concept signed_integer = integer<t_numeric_type> && numeric_signed<t_numeric_type>;

    template <typename t_numeric_type>
    concept arithmetic = std::is_arithmetic_v<t_numeric_type>;

    template <typename t_numeric_type>
    concept signed_arithmetic = arithmetic<t_numeric_type> && numeric_signed<t_numeric_type>;

    template <typename t_numeric_type>
    concept zero_assignable = requires(t_numeric_type& x)
    {
        {x = 0};
    }; // concept zero_assignable

    template <typename t_numeric_type>
    concept one_assignable = requires(t_numeric_type& x)
    {
        {x = 1};
    }; // concept one_assignable

    template <typename t_numeric_type>
    concept zero_comparable = requires(const t_numeric_type& x)
    {
        {x < 0} -> std::same_as<bool>;
        {x > 0} -> std::same_as<bool>;
        {x == 0} -> std::same_as<bool>;
        {x <= 0} -> std::same_as<bool>;
        {x >= 0} -> std::same_as<bool>;
    }; // concept zero_comparable

    template <typename t_numeric_type>
    concept closed_under_binary_and = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x & y} -> std::same_as<t_numeric_type>;
        {z &= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_binary_and

    template <typename t_numeric_type>
    concept closed_under_binary_or = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x | y} -> std::same_as<t_numeric_type>;
        {z |= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_binary_or

    template <typename t_numeric_type>
    concept closed_under_binary_xor = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x ^ y} -> std::same_as<t_numeric_type>;
        {z ^= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_binary_xor

    template <typename t_numeric_type>
    concept binary_mask = zero_assignable<t_numeric_type> && one_assignable<t_numeric_type> &&
        closed_under_binary_and<t_numeric_type> && closed_under_binary_or<t_numeric_type> && closed_under_binary_xor<t_numeric_type> &&
        requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
        {
            {x << 1} -> std::same_as<t_numeric_type>;
            {x >> 1} -> std::same_as<t_numeric_type>;

            {z <<= 1} -> std::same_as<t_numeric_type&>;
            {z >>= 1} -> std::same_as<t_numeric_type&>;
        }; // concept binary_mask

    template <typename t_numeric_type>
    concept closed_under_addition = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x + y} -> std::same_as<t_numeric_type>;
        {z += y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_addition

    template <typename t_numeric_type>
    concept closed_under_subtraction = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x - y} -> std::same_as<t_numeric_type>;
        {z -= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_subtraction

    template <typename t_numeric_type>
    concept closed_under_multiplication = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x * y} -> std::same_as<t_numeric_type>;
        {z *= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_multiplication

    template <typename t_numeric_type>
    concept closed_under_division = requires(const t_numeric_type& x, const t_numeric_type& y, t_numeric_type& z)
    {
        {x / y} -> std::same_as<t_numeric_type>;
        {z /= y} -> std::same_as<t_numeric_type&>;
    }; // concept closed_under_division

    /** The type is closed under addition and subtraction, has an additive identity
     *  element 0, additive inverse for every element of \tparam t_numeric_type, and
     *  if \em x, \em y, \em z, are arbitrary elements of \tparam t_numeric_type, then:
     *  \li x + (y + z) = (x + y) + z
     *  \li x + y = y + x
     *  @todo Consider moving declaration to aftermath::algebra.
     */
    template <typename t_numeric_type>
    concept abelian_group = zero_assignable<t_numeric_type> &&
        closed_under_addition<t_numeric_type> && closed_under_subtraction<t_numeric_type>;

    /** The \c abelian_group \tparam t_numeric_type is closed under multiplication,
     *  has a multiplicative identity element 1, and if \em x, \em y, \em z, are
     *  arbitrary elements of \tparam t_numeric_type, then:
     *  \li x (y z) = (x y) z
     *  \li x 1 = 1 x = x
     *  \li x (y + z) = x y + x z
     *  \li (x + y) z = x z + y z
     *  @todo Consider moving declaration to aftermath::algebra.
     */
    template <typename t_numeric_type>
    concept ring = abelian_group<t_numeric_type> &&
        one_assignable<t_numeric_type> && closed_under_multiplication<t_numeric_type>;

    /** The \c ring \tparam t_numeric_type is closed under subtraction and division,
     *  and has a multiplicative inverse for every element of \tparam t_numeric_type
     *  except for 0.
     *  @todo Consider moving declaration to aftermath::algebra.
     */
    template <typename t_numeric_type>
    concept field = ring<t_numeric_type> && closed_under_division<t_numeric_type>;

    /** Indicates that the pair \tparam t_scalar_type, \tparam t_numeric_type, forms a left module.
     *  More specifically, if \em r, \em s, are arbitrary scalars from the ring \tparam t_scalar_type,
     *  1 is the multiplicative identity of \tparam t_scalar_type, and \em x, \em y, are arbitrary
     *  elements of the abelian group \tparam t_numeric_type, then:
     *  \li r (x + y) = r x + r y
     *  \li (r + s) x = r x + s x
     *  \li (r s) x = r (s x)
     *  \li 1 x = x
     *  @todo Consider moving declaration to aftermath::algebra.
     */
    template <typename t_scalar_type, typename t_numeric_type>
    concept left_module = abelian_group<t_numeric_type> && ring<t_scalar_type> &&
        requires(const t_scalar_type& r, const t_numeric_type& x)
        {
            {r * x} -> std::same_as<t_numeric_type>;
        }; // concept left_module

    /** Indicates that the pair \tparam t_scalar_type, \tparam t_numeric_type, forms a right module.
     *  More specifically, if \em r, \em s, are arbitrary scalars from the ring \tparam t_scalar_type,
     *  1 is the multiplicative identity of \tparam t_scalar_type, and \em x, \em y, are arbitrary
     *  elements of the abelian group \tparam t_numeric_type, then:
     *  \li (x + y) r = x r + y r
     *  \li x (r + s) = x t + x s
     *  \li x (r s) = (x r) s
     *  \li x 1 = x
     *  @todo Consider moving declaration to aftermath::algebra.
     */
    template <typename t_scalar_type, typename t_numeric_type>
    concept right_module = abelian_group<t_numeric_type> && ring<t_scalar_type> &&
        requires(const t_scalar_type& r, const t_numeric_type& x)
        {
            {x * r} -> std::same_as<t_numeric_type>;
        }; // concept right_module
} // namespace ropufu

namespace ropufu
{
    namespace detail
    {
        template <typename t_type>
        struct try_make_signed
        {
            using type = t_type;
        }; // struct ensure_signed

        template <typename t_integer_type>
            requires (!std::numeric_limits<t_integer_type>::is_signed)
        struct try_make_signed<t_integer_type>
        {
            using type = std::make_signed_t<t_integer_type>;
        }; // struct ensure_signed
    } // namespace detail

    /** If the arithmetic type is already signed, defines the type itself;
     *  otherwise defines \c std::make_signed_t<t_arithmetic_type>.
     **/
    template <ropufu::arithmetic t_arithmetic_type>
    using try_make_signed_t = typename detail::try_make_signed<t_arithmetic_type>::type;
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_CONCEPTS_HPP_INCLUDED
