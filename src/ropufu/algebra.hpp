
#ifndef ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED

#include "algebra/elementwise.hpp"
#include "algebra/fraction.hpp"
#include "algebra/matrix.hpp"
#include "algebra/matrix_arrangement.hpp"
#include "algebra/matrix_index.hpp"
#include "algebra/mixed_numeral.hpp"
#include "algebra/permutation.hpp"
#include "algebra/range.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    using elementwise_t = ropufu::aftermath::algebra::elementwise;
    
    /** @brief Represents rational numbers. */
    template <typename t_integer_type>
    using fraction_t = ropufu::aftermath::algebra::fraction<t_integer_type>;
    
    /** @brief A compact way to index matrices. */
    template <typename t_size_type = std::size_t>
    using matrix_index_t = ropufu::aftermath::algebra::matrix_index<t_size_type>;

    /** @brief Row major matrix arrengement. */
    template <typename t_size_type = std::size_t>
    using row_major_t = ropufu::aftermath::algebra::detail::row_major<t_size_type>;

    /** @brief Column major matrix arrengement. */
    template <typename t_size_type = std::size_t>
    using column_major_t = ropufu::aftermath::algebra::detail::column_major<t_size_type>;

    /** @brief A rectangular array. */
    template <typename t_value_type,
        typename t_arrangement_type = ropufu::aftermath::algebra::detail::row_major<typename std::allocator_traits<std::allocator<t_value_type>>::size_type>,
        typename t_allocator_type = std::allocator<t_value_type>>
    using matrix_t = ropufu::aftermath::algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>;

    /** @brief Row major matrix alias with default allocator. */
    template <typename t_value_type>
    using rmatrix_t = ropufu::aftermath::algebra::rmatrix_t<t_value_type>;

    /** @brief Column major matrix alias with default allocator. */
    template <typename t_value_type>
    using cmatrix_t = ropufu::aftermath::algebra::cmatrix_t<t_value_type>;
    
    template <std::size_t t_denominator, typename t_float_type = double>
    using mixed_numeral_t = ropufu::aftermath::algebra::mixed_numeral<t_denominator, t_float_type>;
    
    template <typename t_size_type = std::size_t>
    using permutation_t = ropufu::aftermath::algebra::permutation<t_size_type>;

    using spacing_t = ropufu::aftermath::algebra::spacing;

    template <typename t_value_type>
    using range_t = ropufu::aftermath::algebra::range<t_value_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED
