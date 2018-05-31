
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED

#include <cstddef>     // std::size_t
#include <type_traits> // std::is_same_v

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        /** @brief Describes how a matrix is stored in memory.
          *  @example Consider the following matrix:
          *      || a b c ||
          *      || d e f ||
          *    In row-major format it will be stored as (a b c d e f).
          */
        template <typename t_size_type = std::size_t>
        struct row_major
        {
            using type = row_major;
            using size_type = t_size_type;

            /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
            static size_type flatten(size_type row_index, size_type column_index, size_type /*height*/, size_type width) noexcept
            {
                return row_index * width + column_index;
            } // flatten(...)
        }; // struct row_major

        /** @brief Describes how a matrix is stored in memory.
         *  @example Consider the following matrix:
         *      || a b c ||
         *      || d e f ||
         *    In column-major format it will be stored as (a d b e c f).
         */
        template <typename t_size_type = std::size_t>
        struct column_major
        {
            using type = row_major;
            using size_type = t_size_type;

            /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
            static size_type flatten(size_type row_index, size_type column_index, size_type height, size_type /*width*/) noexcept
            {
                return column_index * height + row_index;
            } // flatten(...)
        }; // struct column_major
    } // namespace detail
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
