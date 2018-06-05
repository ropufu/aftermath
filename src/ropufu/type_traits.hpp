
#ifndef ROPUFU_AFTERMATH_TYPE_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TYPE_TRAITS_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <type_traits> // ...
#include <utility> // std::declval

#define ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(BINOP, OPNAME)                                               \
template <typename, typename, typename = void> struct has_##OPNAME##_assign : public std::false_type { };   \
                                                                                                            \
template <typename t_left_type, typename t_right_type> struct has_##OPNAME##_assign<                        \
    t_left_type, t_right_type, std::void_t<                                                                 \
        decltype( std::declval<t_left_type&>() BINOP std::declval<const t_right_type&>() )                  \
    >> : public std::true_type { };                                                                         \
                                                                                                            \
template <typename t_left_type, typename t_right_type = t_left_type>                                        \
inline constexpr bool has_##OPNAME##_assign_v = has_##OPNAME##_assign<t_left_type, t_right_type>::value;    \
                                                                                                            \


#define ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(BINOP, OPNAME)                                               \
template <typename, typename, typename = void> struct has_##OPNAME##_binary : public std::false_type { };   \
                                                                                                            \
template <typename t_left_type, typename t_right_type> struct has_##OPNAME##_binary<                        \
    t_left_type, t_right_type, std::void_t<                                                                 \
        decltype( std::declval<const t_left_type&>() BINOP std::declval<const t_right_type&>() )            \
    >> : public std::true_type { };                                                                         \
                                                                                                            \
template <typename t_left_type, typename t_right_type = t_left_type>                                        \
inline constexpr bool has_##OPNAME##_binary_v = has_##OPNAME##_binary<t_left_type, t_right_type>::value;    \
                                                                                                            \


#define ROPUFU_AFTERMATH_TYPE_TRAITS_MEMBER_FUNC(FEXPR, FNAME)                                              \
template <typename, typename = void> struct has_##FNAME##_func : public std::false_type { };                \
                                                                                                            \
template <typename t_type> struct has_##FNAME##_func<                                                       \
    t_type, std::void_t<                                                                                    \
        decltype( std::declval<t_type>()FEXPR )                                                             \
    >> : public std::true_type { };                                                                         \
                                                                                                            \
template <typename t_type>                                                                                  \
inline constexpr bool has_##FNAME##_func_v = has_##FNAME##_func<t_type>::value;                             \
                                                                                                            \

namespace ropufu::aftermath
{
    namespace type_traits
    {
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(+=, add)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(-=, subtract)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(*=, multiply)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(/=, divide)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(|=, binor)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(&=, binand)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(^=, binxor)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(<<=, left_shift)
        ROPUFU_AFTERMATH_TYPE_TRAITS_ASSIGN_OP(>>=, right_shift)

        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(==, equality)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(!=, inequality)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(<<, left_shift)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(>>, right_shift)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(>, greater)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(<, less)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(+, add)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(-, subtract)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(*, multiply)
        ROPUFU_AFTERMATH_TYPE_TRAITS_BINARY_OP(/, divide)

        ROPUFU_AFTERMATH_TYPE_TRAITS_MEMBER_FUNC(.begin(), begin)
        ROPUFU_AFTERMATH_TYPE_TRAITS_MEMBER_FUNC(.cbegin(), cbegin)
        ROPUFU_AFTERMATH_TYPE_TRAITS_MEMBER_FUNC(.end(), end)
        ROPUFU_AFTERMATH_TYPE_TRAITS_MEMBER_FUNC(.cend(), cend)


        template <typename, typename = void> struct has_prefix_increment : public std::false_type { };
        template <typename t_type> struct has_prefix_increment<t_type, std::void_t<
            decltype( ++std::declval<t_type&>() )
            >> : public std::true_type { };

        template <typename, typename = void> struct has_prefix_decrement : public std::false_type { };
        template <typename t_type> struct has_prefix_decrement<t_type, std::void_t<
            decltype( ++std::declval<t_type&>() )
            >> : public std::true_type { };

        template <typename t_type> inline constexpr bool has_prefix_increment_v = has_prefix_increment<t_type>::value;
        template <typename t_type> inline constexpr bool has_prefix_decrement_v = has_prefix_decrement<t_type>::value;

        /** @brief Indicates if \tparam t_type can be used in range-based for loops. */
        template <typename t_type>
        inline constexpr bool is_iterable_v = has_begin_func_v<t_type> && has_end_func_v<t_type>;
        
        /** @brief Indicates if \tparam t_type can basic for loops \c for (x = first; x < past_the_last; ++x). */
        template <typename t_type>
        inline constexpr bool is_one_by_one_iterable_v = has_prefix_increment_v<t_type> && has_less_binary_v<t_type, t_type>;
    } // namespace type_traits
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_TYPE_TRAITS_HPP_INCLUDED
