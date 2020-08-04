
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_SLICE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_SLICE_HPP_INCLUDED

#include <algorithm>   // std::copy
#include <cstddef>     // std::size_t
#include <cstring>     // std::memcpy
#include <iterator>    // std::cbegin, std::cend
#include <stdexcept>   // std::logic_error
#include <type_traits> // std::is_trivially_copyable_v, std::conditional_t
#include <utility>     // std::move
#include <vector>      // std::vector

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename t_value_ptr_type, typename t_if_const_type = void, typename t_if_nonconst_type = void>
        struct const_nonconst_pointer_switch
        {
            using type = void;
            using value_type = void;
        }; // struct const_nonconst_pointer_switch

        template <typename t_value_type, typename t_if_const_type, typename t_if_nonconst_type>
        struct const_nonconst_pointer_switch<const t_value_type*, t_if_const_type, t_if_nonconst_type>
        {
            using type = t_if_const_type;
            using value_type = t_value_type;
        }; // struct const_nonconst_pointer_switch<...>

        template <typename t_value_type, typename t_if_const_type, typename t_if_nonconst_type>
        struct const_nonconst_pointer_switch<t_value_type*, t_if_const_type, t_if_nonconst_type>
        {
            using type = t_if_nonconst_type;
            using value_type = t_value_type;
        }; // struct const_nonconst_pointer_switch<...>

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

        template <typename t_value_type, typename t_stride_type, bool t_is_const>
        struct matrix_slice_iterator
        {
            using type = matrix_slice_iterator<t_value_type, t_stride_type, t_is_const>;
            using stride_type = t_stride_type;
            using pointer_type = std::conditional_t<t_is_const, const t_value_type*, t_value_type*>;
            using iterator_result_type = std::conditional_t<t_is_const, const t_value_type&, t_value_type&>;
            using const_iterator_result_type = const t_value_type&;

        private:
            pointer_type m_current_ptr = nullptr;
            const stride_type* m_stride_ptr = nullptr;
            const std::ptrdiff_t* m_step_ptr = nullptr;

        public:
            matrix_slice_iterator(pointer_type current_ptr, const stride_type& stride) noexcept
                : m_current_ptr(current_ptr), m_stride_ptr(&stride), m_step_ptr(stride.data())
            {
            } // matrix_slice_iterator(...)

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
                this->m_current_ptr += *(this->m_step_ptr);
                this->m_stride_ptr->increment(this->m_step_ptr);
                return *this;
            } // operator ++(...)
        }; // struct matrix_slice_iterator
    } // namespace detail
        
    template <typename t_value_type, typename t_stride_type>
    struct matrix_slice;

    template <typename t_value_type, typename t_stride_type>
    struct const_matrix_slice : public detail::matrix_slice_copy_module<const_matrix_slice<t_value_type, t_stride_type>, t_value_type>
    {
        using type = const_matrix_slice<t_value_type, t_stride_type>;
        using stride_type = t_stride_type;
        using size_type = std::size_t;
        using pointer_type = const t_value_type*;
        using const_iterator_type = detail::matrix_slice_iterator<t_value_type, t_stride_type, true>;

        friend detail::matrix_slice_copy_module<type, t_value_type>;
        template <typename, typename> friend struct matrix_slice;

    private:
        pointer_type m_begin_ptr = nullptr;
        pointer_type m_end_ptr = nullptr;
        stride_type m_stride = {};
        size_type m_count = 0;

    public:
        const_matrix_slice(pointer_type begin_ptr, pointer_type end_ptr,
            stride_type&& stride, size_type count) noexcept
            : m_begin_ptr(begin_ptr), m_end_ptr(end_ptr), m_stride(std::move(stride)), m_count(count)
        {
        } // const_matrix_slice(...)

        const_matrix_slice(const type& other) = default;
        const_matrix_slice(type&& other) = default;
        type& operator =(const type& other) = delete;
        type& operator =(type&& other) = delete;

        size_type size() const noexcept { return this->m_count; }
        bool contiguous() const noexcept { return this->m_stride.contiguous(); }
        
        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }
    }; // struct const_matrix_slice
    
    template <typename t_value_type, typename t_stride_type>
    struct matrix_slice
        : public detail::matrix_slice_copy_module<const_matrix_slice<t_value_type, t_stride_type>, t_value_type>,
        public detail::matrix_slice_paste_module<const_matrix_slice<t_value_type, t_stride_type>, t_value_type>
    {
        using type = matrix_slice<t_value_type, t_stride_type>;
        using stride_type = t_stride_type;
        using size_type = std::size_t;
        using pointer_type = t_value_type*;
        using iterator_type = detail::matrix_slice_iterator<t_value_type, t_stride_type, false>;
        using const_iterator_type = detail::matrix_slice_iterator<t_value_type, t_stride_type, true>;

        using const_type = const_matrix_slice<t_value_type, t_stride_type>;
        friend detail::matrix_slice_copy_module<type, t_value_type>;
        friend detail::matrix_slice_paste_module<type, t_value_type>;

    private:
        pointer_type m_begin_ptr = nullptr;
        pointer_type m_end_ptr = nullptr;
        stride_type m_stride = {};
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
        matrix_slice(pointer_type begin_ptr, pointer_type end_ptr,
            stride_type&& stride, size_type count) noexcept
            : m_begin_ptr(begin_ptr), m_end_ptr(end_ptr), m_stride(std::move(stride)), m_count(count)
        {
        } // matrix_slice(...)

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
        bool contiguous() const noexcept { return this->m_stride.contiguous(); }
        
        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_begin_ptr, this->m_stride); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_end_ptr, this->m_stride); }

        iterator_type begin() noexcept { return iterator_type(this->m_begin_ptr, this->m_stride); }
        iterator_type end() noexcept { return iterator_type(this->m_end_ptr, this->m_stride); }
    }; // struct matrix_slice

    template <typename t_value_ptr_type, typename t_stride_type>
    using matrix_slice_t = typename detail::const_nonconst_pointer_switch<t_value_ptr_type,
        const_matrix_slice<typename detail::const_nonconst_pointer_switch<t_value_ptr_type>::value_type, t_stride_type>,
        matrix_slice<typename detail::const_nonconst_pointer_switch<t_value_ptr_type>::value_type, t_stride_type>>::type;
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_SLICE_HPP_INCLUDED
