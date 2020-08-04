
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED

#include "iterator_stride.hpp"
#include "matrix_slice.hpp"

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <type_traits> // std::make_signed_t
#include <utility>     // std::move

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        /** @brief Describes how a matrix is stored in memory. */
        template <typename t_derived_type, typename t_size_type,
            typename t_row_stride_type,
            typename t_column_stride_type,
            typename t_diagonal_stride_type>
        struct matrix_arrangement_core
        {
            using derived_type = t_derived_type;
            using size_type = t_size_type;
            using row_stride_type = t_row_stride_type;
            using column_stride_type = t_column_stride_type;
            using diagonal_stride_type = t_diagonal_stride_type;

            using signed_size_type = std::make_signed_t<size_type>;

        protected:
            size_type m_height = 0;
            size_type m_width = 0;

        public:
            constexpr matrix_arrangement_core() noexcept { }

            constexpr matrix_arrangement_core(size_type height, size_type width) noexcept
                : m_height(height), m_width(width)
            {
            } // matrix_arrangement_core(...)

            /** Height of the matrix. */
            constexpr size_type height() const noexcept { return this->m_height; }

            /** Width of the matrix. */
            constexpr size_type width() const noexcept { return this->m_width; }
                
            /** Number of elements in the matrix. */
            constexpr size_type size() const noexcept { return this->m_height * this->m_width; }
                
            /** Smallest of the width and height of the matrix. */
            constexpr size_type square_size() const noexcept
            {
                return this->m_height < this->m_width ? this->m_height : this->m_width;
            } // square_size(...)

            /** Checks if the matrix is empty. */
            constexpr bool empty() const noexcept { return this->size() == 0; }

            /** Checks if the matrix is a square matrix. */
            constexpr bool square() const noexcept { return this->m_height == this->m_width; }

            /** @brief Re-shape the matrix.
             *  @remark The behavior of this operation may depend on \c arrangement_type of this matrix.
             */
            bool try_reshape(size_type height, size_type width) noexcept
            {
                if (height * width != this->size()) return false;
                this->m_height = height;
                this->m_width = width;
                return true;
            } // reshape(...)

            template <typename t_value_ptr_type>
            auto row_slice(size_type row_index, t_value_ptr_type begin_ptr) const
                -> matrix_slice_t<t_value_ptr_type, row_stride_type>
            {
                if (row_index >= this->m_height) throw std::out_of_range("Row index must be less than the height of the matrix.");                
                const derived_type* that = static_cast<const derived_type*>(this);

                size_type first_index = that->flatten(row_index, 0);
                size_type past_the_last_index = that->flatten(row_index, this->m_width);
                row_stride_type stride = that->row_iterator_stride();
                return { begin_ptr + first_index, begin_ptr + past_the_last_index, std::move(stride), this->m_width };
            } // column_slice(...)

            template <typename t_value_ptr_type>
            auto column_slice(size_type column_index, t_value_ptr_type begin_ptr) const
                -> matrix_slice_t<t_value_ptr_type, column_stride_type>
            {
                if (column_index >= this->m_width) throw std::out_of_range("Column index must be less than the width of the matrix.");
                const derived_type* that = static_cast<const derived_type*>(this);

                size_type first_index = that->flatten(0, column_index);
                size_type past_the_last_index = that->flatten(this->m_height, column_index);
                column_stride_type stride = that->column_iterator_stride();
                return { begin_ptr + first_index, begin_ptr + past_the_last_index, std::move(stride), this->m_height };
            } // column_slice(...)

            template <typename t_value_ptr_type>
            auto diagonal_slice(signed_size_type diagonal_index, t_value_ptr_type begin_ptr) const
                -> matrix_slice_t<t_value_ptr_type, diagonal_stride_type>
            {
                size_type first_row_index = 0;
                size_type first_column_index = 0;
                size_type count = 0;
                if (diagonal_index >= 0) // Upper triangle.
                {
                    first_column_index = static_cast<size_type>(diagonal_index);
                    count = this->m_width - first_column_index;
                } // if (...)
                else // Lower triangle.
                {
                    first_row_index = static_cast<size_type>(-diagonal_index);
                    count = this->m_height - first_row_index;
                } // else (...)

                if (first_row_index > this->m_height) throw std::out_of_range("Diagonal index cannot be less than the negative height of the matrix.");
                if (first_column_index > this->m_width) throw std::out_of_range("Diagonal index cannot be greater than the width of the matrix.");
                const derived_type* that = static_cast<const derived_type*>(this);

                if (count > this->square_size()) count = this->square_size();
                size_type first_index = that->flatten(first_row_index, first_column_index);
                size_type past_the_last_index = that->flatten(first_row_index + count, first_column_index + count);
                diagonal_stride_type stride = that->diagonal_iterator_stride();
                return { begin_ptr + first_index, begin_ptr + past_the_last_index, std::move(stride), count };
            } // column_slice(...)
        }; // struct matrix_arrangement_core
    } // namespace detail

    /** @brief Describes how a matrix is stored in memory.
         *  @example Consider the following matrix:
         *      || a b c ||
         *      || d e f ||
         *    In row-major format it will be stored as (a b c d e f).
         */
    template <typename t_size_type = std::size_t>
    struct row_major : public detail::matrix_arrangement_core<row_major<t_size_type>, t_size_type,
        detail::iterator_fixed_stride,
        detail::iterator_fixed_stride,
        detail::iterator_fixed_stride>
    {
        using type = row_major<t_size_type>;
        using size_type = t_size_type;
        
        using row_stride_type = detail::iterator_fixed_stride;
        using column_stride_type = detail::iterator_fixed_stride;
        using diagonal_stride_type = detail::iterator_fixed_stride;

        using base_type = detail::matrix_arrangement_core<type, size_type,
            row_stride_type,
            column_stride_type,
            diagonal_stride_type>;

        template <typename, typename, typename, typename, typename>
        friend struct detail::matrix_arrangement_core;

    protected:
        /** Slice stride used when iterating over a given row. */
        constexpr row_stride_type row_iterator_stride() const noexcept { return 1; }
        /** Slice stride used when iterating over a given column. */
        column_stride_type column_iterator_stride() const noexcept { return this->m_width; }
        /** Slice stride used when iterating over a given diagonal. */
        diagonal_stride_type diagonal_iterator_stride() const noexcept { return this->m_width + 1; }

    public:
        using base_type::matrix_arrangement_core; // Inherit constructors.

        /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
        size_type flatten(size_type row_index, size_type column_index) const noexcept
        {
            return row_index * this->m_width + column_index;
        } // flatten(...)

        /** Translates a 1-dimensional index into a 2-dimentional index. Does not perform range validation. */
        void reconstruct(size_type flat_index, size_type& row_index, size_type& column_index) const noexcept
        {
            column_index = flat_index % this->m_width;
            row_index = (flat_index - column_index) / this->m_width;
        } // reconstruct(...)
    }; // struct row_major

    /** @brief Describes how a matrix is stored in memory.
     *  @example Consider the following matrix:
     *      || a b c ||
     *      || d e f ||
     *    In column-major format it will be stored as (a d b e c f).
     */
    template <typename t_size_type = std::size_t>
    struct column_major : public detail::matrix_arrangement_core<column_major<t_size_type>, t_size_type,
        detail::iterator_fixed_stride,
        detail::iterator_fixed_stride,
        detail::iterator_fixed_stride>
    {
        using type = column_major<t_size_type>;
        using size_type = t_size_type;
        
        using row_stride_type = detail::iterator_fixed_stride;
        using column_stride_type = detail::iterator_fixed_stride;
        using diagonal_stride_type = detail::iterator_fixed_stride;

        using base_type = detail::matrix_arrangement_core<type, size_type,
            row_stride_type,
            column_stride_type,
            diagonal_stride_type>;

        template <typename, typename, typename, typename, typename>
        friend struct detail::matrix_arrangement_core;

    protected:
        /** Slice stride used when iterating over a given row. */
        row_stride_type row_iterator_stride() const noexcept { return this->m_height; }
        /** Slice stride used when iterating over a given column. */
        constexpr column_stride_type column_iterator_stride() const noexcept { return 1; }
        /** Slice stride used when iterating over a given diagonal. */
        diagonal_stride_type diagonal_iterator_stride() const noexcept { return this->m_height + 1; }

    public:
        using base_type::matrix_arrangement_core; // Inherit constructors.

        /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
        size_type flatten(size_type row_index, size_type column_index) const noexcept
        {
            return column_index * this->m_height + row_index;
        } // flatten(...)

        /** Translates a 1-dimensional index into a 2-dimentional index. Does not perform range validation. */
        void reconstruct(size_type flat_index, size_type& row_index, size_type& column_index) const noexcept
        {
            row_index = flat_index % this->m_height;
            column_index = (flat_index - row_index) / this->m_height;
        } // reconstruct(...)
    }; // struct column_major
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
