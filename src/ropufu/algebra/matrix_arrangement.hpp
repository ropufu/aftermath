
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED

#include <algorithm>   // std::copy
#include <cstddef>     // std::size_t
#include <cstring>     // std::memcpy
#include <iterator>    // std::cbegin, std::cend
#include <stdexcept>   // std::logic_error
#include <type_traits> // ...

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename t_derived_type, typename t_value_type>
        struct matrix_slice_copy_module
        {
            void copy(t_value_type* destination) const
            {
                const t_derived_type& self = static_cast<const t_derived_type&>(*this);
                if constexpr (!std::is_trivially_copyable_v<t_value_type>) std::copy(self.begin(), self.end(), destination);
                else
                {
                    if (self.contiguous()) std::memcpy(destination, self.m_begin_ptr, static_cast<std::size_t>(self.m_count) * sizeof(t_value_type));
                    else std::copy(self.begin(), self.end(), destination);
                } // if constexpr (...)
            } // copy(...)

            template <typename t_container_type>
            void copy(t_container_type& destination) const
            {
                const t_derived_type& self = static_cast<const t_derived_type&>(*this);
                std::copy(self.begin(), self.end(), destination.begin());
            } // copy(...)
        }; // struct matrix_slice_copy_module
        
        template <typename t_derived_type, typename t_value_type>
        struct matrix_slice_paste_module
        {
            void paste(const t_value_type* source, std::size_t count)
            {
                const t_derived_type& self = static_cast<const t_derived_type&>(*this);
                if constexpr (!std::is_trivially_copyable_v<t_value_type>) std::copy(std::cbegin(source), std::cend(source), self.begin());
                else
                {
                    if (self.contiguous()) std::memcpy(self.m_begin_ptr, source, count * sizeof(t_value_type));
                    else std::copy(std::cbegin(source), std::cend(source), self.begin());
                } // if constexpr (...)
            } // paste(...)

            template <typename t_container_type>
            void paste(const t_container_type& source)
            {
                const t_derived_type& self = static_cast<const t_derived_type&>(*this);
                std::copy(source.begin(), source.end(), self.begin());
            } // paste(...)
        }; // struct matrix_slice_paste_module

        template <typename t_value_type, typename t_size_type, bool t_is_const>
        struct matrix_slice_iterator
        {
            using type = matrix_slice_iterator<t_value_type, t_size_type, t_is_const>;
            using size_type = t_size_type;
            using pointer_type = std::conditional_t<t_is_const, const t_value_type*, t_value_type*>;
            using iterator_result_type = std::conditional_t<t_is_const, const t_value_type&, t_value_type&>;
            using const_iterator_result_type = const t_value_type&;

        private:
            pointer_type m_current_ptr = nullptr;
            size_type m_stride = 0;

        public:
            matrix_slice_iterator(pointer_type current_ptr, size_type stride) noexcept : m_current_ptr(current_ptr), m_stride(stride) { }

            /** Inequality operator, used as termination condition. */
            bool operator !=(const type& other) const noexcept { return this->m_current_ptr != other.m_current_ptr; }
            /** Equality operator. */
            bool operator ==(const type& other) const noexcept { return this->m_current_ptr == other.m_current_ptr; }

            /** Returns the value at current position. */
            const_iterator_result_type operator *() const noexcept { return *this->m_current_ptr; }
            /** Returns the value at current position. */
            iterator_result_type operator *() noexcept { return *this->m_current_ptr; }

            /** Move to the next element. No boundary checks are performed! */
            type& operator ++() noexcept
            {
                this->m_current_ptr += this->m_stride;
                return *this;
            } // operator ++(...)
        }; // struct matrix_slice_iterator
        
        template <typename t_value_type, typename t_size_type>
        struct matrix_slice;

        template <typename t_value_type, typename t_size_type>
        struct const_matrix_slice : public matrix_slice_copy_module<const_matrix_slice<t_value_type, t_size_type>, t_value_type>
        {
            using type = const_matrix_slice<t_value_type, t_size_type>;
            using size_type = t_size_type;
            using pointer_type = const t_value_type*;
            using const_iterator_type = matrix_slice_iterator<t_value_type, t_size_type, true>;

            friend struct matrix_slice_copy_module<type, t_value_type>;
            template <typename, typename> friend struct matrix_slice;

        private:
            pointer_type m_begin_ptr = nullptr;
            pointer_type m_end_ptr = nullptr;
            size_type m_stride = 0;
            size_type m_count = 0;

        public:
            const_matrix_slice(pointer_type begin_ptr, pointer_type end_ptr, size_type stride, size_type count) noexcept : m_begin_ptr(begin_ptr), m_end_ptr(end_ptr), m_stride(stride), m_count(count) { }
            const_matrix_slice(const type& other) = default;
            const_matrix_slice(type&& other) = default;
            type& operator =(const type& other) = delete;
            type& operator =(type&& other) = delete;

            size_type size() const noexcept { return this->m_count; }
            bool contiguous() const noexcept { return this->m_stride == 1; }
            
            const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
            const_iterator_type cend() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

            const_iterator_type begin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
            const_iterator_type end() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }
        }; // struct const_matrix_slice
        
        template <typename t_value_type, typename t_size_type>
        struct matrix_slice
            : public matrix_slice_copy_module<const_matrix_slice<t_value_type, t_size_type>, t_value_type>,
            public matrix_slice_paste_module<const_matrix_slice<t_value_type, t_size_type>, t_value_type>
        {
            using type = matrix_slice<t_value_type, t_size_type>;
            using size_type = t_size_type;
            using pointer_type = t_value_type*;
            using iterator_type = matrix_slice_iterator<t_value_type, t_size_type, false>;
            using const_iterator_type = matrix_slice_iterator<t_value_type, t_size_type, true>;

            using const_type = const_matrix_slice<t_value_type, t_size_type>;
            friend struct matrix_slice_copy_module<type, t_value_type>;
            friend struct matrix_slice_paste_module<type, t_value_type>;

        private:
            pointer_type m_begin_ptr = nullptr;
            pointer_type m_end_ptr = nullptr;
            size_type m_stride = 0;
            size_type m_count = 0;

            template <typename t_other_type>
            type& overwrite_with(const t_other_type& other) noexcept
            {
                iterator_type left_it = this->begin();
                const_iterator_type right_it = other.cbegin();
                for (size_type k = 0; k < this->m_count; ++k)
                {
                    (*left_it) = (*right_it);
                    ++left_it;
                    ++right_it;
                } // for (...)
                return *this;
            } // overwrite(...)

        public:
            matrix_slice(pointer_type begin_ptr, pointer_type end_ptr, size_type stride, size_type count) noexcept : m_begin_ptr(begin_ptr), m_end_ptr(end_ptr), m_stride(stride), m_count(count) { }
            matrix_slice(const type& other) = default;
            matrix_slice(type&& other) = default;

            /** Overwrites the matrix slice with values from \p other. */
            type& operator =(const type& other)
            {
                if (this == &other) return *this; // Do nothing if this is self-assignment.
                if (this->m_count != other.m_count) throw std::logic_error("Matrix slices incompatible.");
                return this->overwrite_with(other);
            } // operator =(...)

            /** Overwrites the matrix slice with values from \p other. */
            type& operator =(const const_type& other)
            {
                if (this->m_count != other.m_count) throw std::logic_error("Matrix slices incompatible.");
                return this->overwrite_with(other);
            } // operator =(...)

            size_type size() const noexcept { return this->m_count; }
            bool contiguous() const noexcept { return this->m_stride == 1; }
            
            const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
            const_iterator_type cend() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

            const_iterator_type begin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
            const_iterator_type end() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

            iterator_type begin() noexcept { return iterator_type(this->m_begin_ptr, this->m_stride); }
            iterator_type end() noexcept { return iterator_type(this->m_end_ptr, this->m_stride); }
        }; // struct matrix_slice

        /** @brief Describes how a matrix is stored in memory.
          *  @example Consider the following matrix:
          *      || a b c ||
          *      || d e f ||
          *    In row-major format it will be stored as (a b c d e f).
          */
        template <typename t_size_type = std::size_t>
        struct row_major
        {
            using type = row_major<t_size_type>;
            using size_type = t_size_type;
            using signed_size_type = std::make_unsigned_t<t_size_type>;

            /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
            static size_type flatten(size_type row_index, size_type column_index, size_type /*height*/, size_type width) noexcept
            {
                return row_index * width + column_index;
            } // flatten(...)

            /** Slice stride used when iterating over a given column. */
            static size_type column_iterator_stride(size_type /*height*/, size_type width) noexcept { return width; }
            /** Slice stride used when iterating over a given row. */
            static constexpr size_type row_iterator_stride(size_type /*height*/, size_type /*width*/) noexcept { return 1; }
            /** Slice stride used when iterating over a given diagonal. */
            static size_type diagonal_iterator_stride(size_type /*height*/, size_type width) noexcept { return width + 1; }
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
            using type = column_major<t_size_type>;
            using size_type = t_size_type;
            using signed_size_type = std::make_unsigned_t<t_size_type>;

            /** Translates a 2-dimensional index into a 1-dimentional index. Does not perform range validation. */
            static size_type flatten(size_type row_index, size_type column_index, size_type height, size_type /*width*/) noexcept
            {
                return column_index * height + row_index;
            } // flatten(...)

            /** Slice stride used when iterating over a given column. */
            static constexpr size_type column_iterator_stride(size_type /*height*/, size_type /*width*/) noexcept { return 1; }
            /** Slice stride used when iterating over a given row. */
            static size_type row_iterator_stride(size_type height, size_type /*width*/) noexcept { return height; }
            /** Slice stride used when iterating over a given diagonal. */
            static size_type diagonal_iterator_stride(size_type height, size_type /*width*/) noexcept { return height + 1; }
        }; // struct column_major
    } // namespace detail
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_ARRANGEMENT_HPP_INCLUDED
