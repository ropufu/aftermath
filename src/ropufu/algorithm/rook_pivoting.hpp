
#ifndef ROPUFU_AFTERMATH_ALGORITHM_ROOK_PIVOTING_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_ROOK_PIVOTING_HPP_INCLUDED

#include "../algebra/matrix_index.hpp"

#include <cmath>    // std::abs
#include <concepts> // std::floating_point
#include <cstddef>  // std::size_t

namespace ropufu::aftermath::algorithm
{
    namespace detail
    {
        template <std::floating_point t_value_type>
        struct indexed
        {
            t_value_type value;
            std::size_t index;
        }; // struct indexed
    } // namespace detail

    /** Column-first rook pivoting strategy for Gaussin Elimination. */
    template <std::floating_point t_value_type>
    struct rook_pivoting
    {
        using type = rook_pivoting<t_value_type>;
        using value_type = t_value_type;

        using indexed_value_type = detail::indexed<value_type>;
        using matrix_index_type = algebra::matrix_index<std::size_t>;

    private:
        template <typename t_matrix_type>
        static constexpr indexed_value_type column_maximum(const t_matrix_type& mat, std::size_t row_start_index, std::size_t column_index) noexcept
        {
            indexed_value_type result {std::abs(mat(row_start_index, column_index)), row_start_index};
            for (std::size_t i = row_start_index + 1; i < mat.height(); ++i)
            {
                value_type x = std::abs(mat(i, column_index));
                if (x > result.value)
                {
                    result.value = x;
                    result.index = i;
                } // if (...)
            } // for (...)
            return result;
        } // column_maximum(...)
        
        template <typename t_matrix_type>
        static constexpr indexed_value_type row_maximum(const t_matrix_type& mat, std::size_t row_index, std::size_t column_start_index) noexcept
        {
            indexed_value_type result {std::abs(mat(row_index, column_start_index)), column_start_index};
            for (std::size_t j = column_start_index + 1; j < mat.width(); ++j)
            {
                value_type x = std::abs(mat(row_index, j));
                if (x > result.value)
                {
                    result.value = x;
                    result.index = j;
                } // if (...)
            } // for (...)
            return result;
        } // row_maximum(...)

    public:
        /** Finds a pivot element in the lower portion of the table.
         *  This element will be a non-zero maximal element in its row and column,
         *  unless the matrix is degenerate and all lower portion elements are 0.
         *   +-----------------------+
         *   |xxxxxxxxxxxxxxxxxxxxxxx|
         *   |xxxx+------------------+
         *   |xxxx|                  |
         *   |xxxx|        ??        |
         *   |xxxx|                  |
         *   +----+------------------+
         *  @remark Will return (step_index, step_index) if both row and
         *  column elements are zeros (in the lower region).
         */
        template <typename t_matrix_type>
        matrix_index_type operator ()(const t_matrix_type& mat, std::size_t step_index) const noexcept
        {
            std::size_t m = mat.height();
            std::size_t n = mat.width();
            std::size_t s = (m < n) ? (m) : (n);

            matrix_index_type result {step_index, step_index};
            while (step_index < s)
            {
                // First step: find a maximal element in current column.
                indexed_value_type column_max = type::column_maximum(mat, step_index, step_index);
                indexed_value_type row_max {-1, 0};
                while (row_max.value < column_max.value)
                {
                    result.row = column_max.index; // Move to the row containing the element located.
                    // Next step: see if the element is maximal in its row as well.
                    row_max = type::row_maximum(mat, result.row, step_index);
                    if (row_max.value == column_max.value) break; // We've found a maximal element.

                    result.column = row_max.index; // Move over to a largest element in this row.
                    column_max = type::column_maximum(mat, step_index, result.column);
                } // while (...)

                if (row_max.value > 0) [[likely]] break;
                
                // This is a degenerate matrix; continue search.
                if (step_index == s - 1) break; // There is nothing we can do.
                ++step_index;
                ++result.row;
                ++result.column;
            } // while (...)
            return result;
        } // operator ()(...)
    }; // struct rook_pivoting
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_ROOK_PIVOTING_HPP_INCLUDED
