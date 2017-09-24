
#ifndef ROPUFU_AFTERMATH_TYPE_CONCEPTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TYPE_CONCEPTS_HPP_INCLUDED

// This file is distributed under the MIT License. See the accompanying file
// LICENSE.txt or http://www.opensource.org/licenses/mit-license.php for terms
// and conditions.

// Taken from https://code.google.com/p/origin/source/browse/trunk/origin/type/concepts.impl/traits.hpp.

#include <functional> 
#include <type_traits>
#include <utility>

#define ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(BINOP, OPNAME)                  \
    template <typename S, typename T = S>                                     \
    struct get_##OPNAME##_result                                              \
    {                                                                         \
    private:                                                                  \
        template <typename X, typename Y>                                     \
        static auto check(X& x, Y& y) -> decltype(x BINOP y);                 \
                                                                              \
        static std::false_type check(...);                                    \
                                                                              \
    public:                                                                   \
        using type = decltype(check(std::declval<S&>(), std::declval<T&>())); \
    };                                                                        \
                                                                              \
    template <typename S, typename T = S>                                     \
    struct has_##OPNAME : public std::integral_constant<bool, !std::is_same<  \
        typename get_##OPNAME##_result<S, T>::type, std::false_type>::value>  \
    {                                                                         \
    };                                                                        \

namespace ropufu
{
    namespace aftermath
    {
        namespace type_impl
        {
            /** Type switcher, depending on the value of \tparam t_condition. */
            template <bool t_condition, typename t_true_type, typename t_false_type>
            struct type_switch
            {
                typedef t_false_type type;
            };

            /** Type switcher when \tparam t_condition is \c true. */
            template <typename t_true_type, typename t_false_type>
            struct type_switch<true, t_true_type, t_false_type>
            {
                typedef t_true_type type;
            };

            /** Shorthand notation for type switcher. */
            template <bool t_condition, typename t_true_type, typename t_false_type>
            using type_switch_t = typename type_switch<t_condition, t_true_type, t_false_type>::type;


            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(== , equality);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(<< , left_shift);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(> , greater);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(< , less);
            
            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(+ , sum);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(- , difference);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(* , product);

            ROPUFU_AFTERMATH_TYPE_CONCEPTS_BINARY(/ , divide);

            
            /** A type trait that returns the result of the ++(...) operator.  */
            template <typename S>
            struct get_increment_result
            {
            private:
                template <typename X>
                static auto check(X& x) -> decltype(++x);

                static std::false_type check(...);

            public:
                using type = decltype(check(std::declval<S&>()));
            };

            /** A type trait that determines if ++(...) is valid. */
            template <typename S>
            struct has_increment : public std::integral_constant<bool, 
                !std::is_same<typename get_increment_result<S>::type, std::false_type>::value>
            {
            };

            /** A type trait that returns the result of the \c std::hash.  */
            template <typename S>
            struct get_hash_result
            {
            private:
                template <typename X>
                static auto check(X& x) -> decltype(std::hash<X>()(x));

                static std::false_type check(...);

            public:
                using type = decltype(check(std::declval<S&>()));
            };

            // A type trait that determines if \c std::hash is properly specialized.
            template <typename S>
            struct has_hash : public std::integral_constant<bool,
                !std::is_same<typename get_hash_result<S>::type, std::false_type>::value>
            {
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_TYPE_CONCEPTS_HPP_INCLUDED
