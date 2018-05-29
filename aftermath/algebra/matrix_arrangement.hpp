
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED

#include "matrix_index.hpp"

#include <cstddef>   // std::size_t
#include <stdexcept> // std::out_of_range

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <bool t_is_row_major = true, bool t_noexcept = true>
        struct matrix_arrangement;

        /** @brief Describes how a matrix is stored in memory.
          *  @example Consider the following matrix:
          *      || a b c ||
          *      || d e f ||
          *    In row-major format it will be stored as (a b c d e f).
          */
        template <bool t_noexcept>
        struct matrix_arrangement<true, t_noexcept>
        {
            /** Row-major format. */
            static std::size_t flatten(std::size_t row_index, std::size_t column_index, std::size_t height, std::size_t width) noexcept(t_noexcept)
            {
                if constexpr (!t_noexcept)
                {
                    if (row_index >= height) throw std::out_of_range("Row index must be less than the height of the matrix.");
                    if (column_index >= width) throw std::out_of_range("Column index must be less than the width of the matrix.");
                } // if constexpr (...)
                return row_index * width + column_index;
            } // flatten(...)

            /** Row-major format. */
            static std::size_t flatten(const matrix_index& index, std::size_t height, std::size_t width) noexcept(t_noexcept)
            {
                if constexpr (!t_noexcept)
                {
                    if (index.row >= height) throw std::out_of_range("Row index must be less than the height of the matrix.");
                    if (index.column >= width) throw std::out_of_range("Column index must be less than the width of the matrix.");
                } // if constexpr (...)
                return index.row * width + index.column;
            } // flatten(...)
        }; // struct matrix_arrangement<...>

        /** @brief Describes how a matrix is stored in memory.
         *  @example Consider the following matrix:
         *      || a b c ||
         *      || d e f ||
         *    In column-major format it will be stored as (a d b e c f).
         */
        template <bool t_noexcept>
        struct matrix_arrangement<false, t_noexcept>
        {
            /** Column-major format. */
            static std::size_t flatten(std::size_t row_index, std::size_t column_index, std::size_t height, std::size_t width) noexcept(t_noexcept)
            {
                if constexpr (!t_noexcept)
                {
                    if (row_index >= height) throw std::out_of_range("Row index must be less than the height of the matrix.");
                    if (column_index >= width) throw std::out_of_range("Column index must be less than the width of the matrix.");
                } // if constexpr (...)
                return column_index * height + row_index;
            } // flatten(...)

            /** Column-major format. */
            static std::size_t flatten(const matrix_index& index, std::size_t height, std::size_t width) noexcept(t_noexcept)
            {
                if constexpr (!t_noexcept)
                {
                    if (index.row >= height) throw std::out_of_range("Row index must be less than the height of the matrix.");
                    if (index.column >= width) throw std::out_of_range("Column index must be less than the width of the matrix.");
                } // if constexpr (...)
                return index.column * height + index.row;
            } // flatten(...)
        }; // struct matrix_arrangement<...>
    } // namespace detail
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
