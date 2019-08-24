
#ifndef ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED

#include "algebra/elementwise.hpp"
#include "algebra/fraction.hpp"
#include "algebra/matrix_arrangement.hpp"
#include "algebra/matrix_index.hpp"
#include "algebra/matrix.hpp"
#include "algebra/range_spacing.hpp"
#include "algebra/range.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    using elementwise_t = aftermath::algebra::elementwise;
    
    /** @brief Represents a rational number (fraction of two integers). */
    template <typename t_integer_type>
    using fraction_t = aftermath::algebra::fraction<t_integer_type>;
    
    /** @brief A compact way to subscript matrices. */
    template <typename t_size_type = aftermath::algebra::matrix_index<>::size_type>
    using matrix_index_t = aftermath::algebra::matrix_index<t_size_type>;

    /** @brief Row major matrix arrengement. */
    template <typename t_size_type = aftermath::algebra::detail::row_major<>::size_type>
    using row_major_t = aftermath::algebra::detail::row_major<t_size_type>;

    /** @brief Column major matrix arrengement. */
    template <typename t_size_type = aftermath::algebra::detail::column_major<>::size_type>
    using column_major_t = aftermath::algebra::detail::column_major<t_size_type>;

    /** @brief A rectangular array. */
    template <typename t_value_type,
        typename t_arrangement_type = typename aftermath::algebra::matrix<t_value_type>::arrangement_type,
        typename t_allocator_type = typename aftermath::algebra::matrix<t_value_type, t_arrangement_type>::allocator_type>
    using matrix_t = aftermath::algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>;

    /** @brief Row major matrix with default allocator. */
    template <typename t_value_type>
    using rmatrix_t = aftermath::algebra::rmatrix_t<t_value_type>;

    /** @brief Column major matrix with default allocator. */
    template <typename t_value_type>
    using cmatrix_t = aftermath::algebra::cmatrix_t<t_value_type>;

    template <typename t_value_type,
        typename t_intermediate_type = typename aftermath::algebra::linear_spacing<t_value_type>::intermediate_type>
    using linear_spacing_t = aftermath::algebra::linear_spacing<t_value_type, t_intermediate_type>;

    template <typename t_value_type,
        typename t_intermediate_type = typename aftermath::algebra::logarithmic_spacing<t_value_type>::intermediate_type>
    using logarithmic_spacing_t = aftermath::algebra::logarithmic_spacing<t_value_type, t_intermediate_type>;

    template <typename t_value_type,
        typename t_intermediate_type = typename aftermath::algebra::exponential_spacing<t_value_type>::intermediate_type>
    using exponential_spacing_t = aftermath::algebra::exponential_spacing<t_value_type, t_intermediate_type>;

    template <typename t_value_type>
    using range_t = aftermath::algebra::range<t_value_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_ALGEBRA_HPP_INCLUDED
