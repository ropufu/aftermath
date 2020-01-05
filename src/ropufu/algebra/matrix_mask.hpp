
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_MASK_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_MASK_HPP_INCLUDED

#include "iterator_stride.hpp"
#include "matrix_arrangement.hpp"
#include "matrix_index.hpp"
#include "matrix_slice.hpp"
#include "../simple_vector.hpp"

#include <cstddef>   // std::size_t, std::ptrdiff_t
#include <memory>    // std::allocator, std::allocator_traits
#include <stdexcept> // std::out_of_range
#include <utility>   // std::move
#include <vector>    // std::vector

namespace ropufu::aftermath::algebra
{
    /** @brief A rectangular array of boolean values. */
    template <typename t_allocator_type = std::allocator<bool>,
        typename t_arrangement_type = row_major<typename std::allocator_traits<t_allocator_type>::size_type>>
    struct matrix_mask;

    template <typename t_allocator_type, typename t_arrangement_type>
    struct matrix_mask
    {
        using type = matrix_mask<t_allocator_type, t_arrangement_type>;
        using value_type = bool;
        using allocator_type = t_allocator_type;
        using arrangement_type = t_arrangement_type;

        using container_type = simple_vector<bool, allocator_type>; // Prevents storage optimization of std::vector<bool>.
        using size_type = typename container_type::size_type;
        using index_type = matrix_index<size_type>;
        
        using iterator_type = typename container_type::iterator_type;
        using const_iterator_type = typename container_type::const_iterator_type;
        using stride_type = detail::iterator_seq_stride;

    private:
        container_type m_container = {}; // Flat data.
        arrangement_type m_arrangement = {}; // Dimensions and structure of the matrix.

    public:
        /** @brief Creates a matrix by stealing from \p other. */
        matrix_mask(type&& other) noexcept
            : m_container(std::move(other.m_container)),
            m_arrangement(other.m_arrangement)
        {
        } // matrix_mask(...)

        /** @brief Copies a matrix by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->m_container = std::move(other.m_container);
            this->m_arrangement = other.m_arrangement;
            return *this;
        } // operator =(...)

        /** @brief Creates an empty matrix. */
        matrix_mask() noexcept { }
        
        /** @brief Create a mask where every element is either
         *  marked (if \p value = true) or unmarked (if \p value = false). */
        matrix_mask(size_type height, size_type width, bool value = false) noexcept
            : m_container(height * width, value), m_arrangement(height, width)
        {
        } // matrix_mask(...)

        /** Height of the matrix. */
        size_type height() const noexcept { return this->m_arrangement.height(); }

        /** Width of the matrix. */
        size_type width() const noexcept { return this->m_arrangement.width(); }
                
        /** Number of elements in the matrix. */
        size_type size() const noexcept { return this->m_arrangement.size(); }

        void set(size_type row_index, size_type column_index)
        {
            size_type index = this->m_arrangement.flatten(row_index, column_index);
            this->m_container[index] = true;
        } // set(...)

        void reset(size_type row_index, size_type column_index)
        {
            size_type index = this->m_arrangement.flatten(row_index, column_index);
            this->m_container[index] = false;
        } // reset(...)

        void flip(size_type row_index, size_type column_index)
        {
            size_type index = this->m_arrangement.flatten(row_index, column_index);
            bool& x = this->m_container[index];
            x = !x;
        } // flip(...)
        
        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator ()(size_type row_index, size_type column_index) const
        {
            return this->m_container[this->m_arrangement.flatten(row_index, column_index)];
        } // operator ()(...)
        
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator ()(size_type row_index, size_type column_index)
        {
            return this->m_container[this->m_arrangement.flatten(row_index, column_index)];
        } // operator ()(...)
        
        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator ()(const index_type& index) const
        {
            return this->m_container[this->m_arrangement.flatten(index.row, index.column)];
        } // operator ()(...)
        
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator ()(const index_type& index)
        {
            return this->m_container[this->m_arrangement.flatten(index.row, index.column)];
        } // operator ()(...)

        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator [](const index_type& index) const { return this->operator ()(index.row, index.column); }

        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator [](const index_type& index) { return this->operator ()(index.row, index.column); }

        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        const value_type& at(size_type row_index, size_type column_index) const
        {
            if (row_index >= this->m_arrangement.height()) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (column_index >= this->m_arrangement.width()) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        } // at(...)

        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(size_type row_index, size_type column_index)
        {
            if (row_index >= this->m_arrangement.height()) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (column_index >= this->m_arrangement.width()) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        } // at(...)

        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        const value_type& at(const index_type& index) const
        {
            if (index.row >= this->m_arrangement.height()) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (index.column >= this->m_arrangement.width()) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(index);
        } // at(...)

        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(const index_type& index)
        {
            if (index.row >= this->m_arrangement.height()) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (index.column >= this->m_arrangement.width()) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(index);
        } // at(...)

        /** Checks whether dimensions of the two matrices are the same. */
        static bool compatible(const type& left, const type& right) noexcept
        {
            return
                (left.m_arrangement.height() == right.m_arrangement.height()) &&
                (left.m_arrangement.width() == right.m_arrangement.width());
        } // compatible(...)

        const_iterator_type cbegin() const noexcept { return this->m_container.cbegin(); }
        const_iterator_type cend() const noexcept { return this->m_container.cend(); }

        const_iterator_type begin() const noexcept { return this->m_container.begin(); }
        const_iterator_type end() const noexcept { return this->m_container.end(); }

        iterator_type begin() noexcept { return this->m_container.begin(); }
        iterator_type end() noexcept { return this->m_container.end(); }

        template <typename t_value_ptr_type>
        auto slice(t_value_ptr_type begin_ptr) const noexcept
            -> matrix_slice_t<t_value_ptr_type, stride_type>
        {
            std::ptrdiff_t offset = 0;
            std::vector<std::ptrdiff_t> steps {};
            steps.reserve(this->m_arrangement.size() + 1);
            
            // Find the first marked element.
            for (const bool* it = this->m_container.cbegin(); it != this->m_container.cend(); ++it)
            {
                if (*it) break;
                ++offset;
            } // for (...)
            
            std::ptrdiff_t k = offset;
            // Pinpoint all marked elements.
            for (const bool* it = this->m_container.cbegin() + offset; it != this->m_container.cend(); ++it)
            {
                if (*it) steps.push_back(k);
                ++k;
            } // for (...)
            size_type count = static_cast<size_type>(steps.size());
            steps.push_back(k); // Terminus: past-the-last index.

            // Now that \c steps contains indices of the marked elements, calculate the differences.
            for (std::size_t i = 0; i < count; ++i) steps[i] = steps[i + 1] - steps[i];
            // The last value of steps, k, will never be used.

            stride_type stride { std::move(steps) };
            return { begin_ptr + offset, begin_ptr + k, std::move(stride), count };
        } // column_slice(...)
    }; // struct matrix_mask
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_MASK_HPP_INCLUDED
