
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
        decltype( std::declval<t_left_type&>() BINOP std::declval<const t_right_type&>() )                 \
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
    } // namespace type_traits
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_TYPE_TRAITS_HPP_INCLUDED
