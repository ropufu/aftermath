
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "matrix_arrangement.hpp"
#include "matrix_index.hpp"

#include <cstddef> // std::size_t
#include <cstring> // std::memcpy, std::memset.
#include <new>
#include <string>
#include <type_traits>
#include <vector>

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename t_data_pointer_type>
        struct matrix_pointer_to_reference;
        
        template <typename t_data_type>
        struct matrix_pointer_to_reference<t_data_type*>
        {
            using reference_type = t_data_type&;
        }; // struct matrix_pointer_to_reference<...>
        
        template <typename t_data_type>
        struct matrix_pointer_to_reference<const t_data_type*>
        {
            using reference_type = const t_data_type&;
        }; // struct matrix_pointer_to_reference<...>
        
        /** An iterator to allow \c matrix to be used in range-based for-loops. */
        template <typename t_data_type, typename t_data_pointer_type>
        struct matrix_iterator
        {
            using type = matrix_iterator<t_data_type, t_data_pointer_type>;
            using data_type = typename matrix_pointer_to_reference<t_data_pointer_type>::reference_type;
            using data_pointer_type = t_data_pointer_type;

        private:
            data_pointer_type m_data_pointer = nullptr;
            std::size_t m_size = 0;
            std::size_t m_position = 0;

        public:
            matrix_iterator(data_pointer_type data_pointer, std::size_t size, std::size_t position) noexcept
                : m_data_pointer(data_pointer), m_size(size), m_position(position)
            {
            } // matrix_iterator(...)

            /** Termination condition. */
            bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }
            /** Equality operator. */
            bool operator ==(const type& other) const noexcept { return this->m_position == other.m_position; }

            /** Returns the current matrix element. Behavior undefined if iterator has reached the end of the collection. */
            data_type operator *() /*const*/ noexcept
            {
                return this->m_data_pointer[this->m_position];
            } // operator *(...)

            /** If not at the end, advances the position of the iterator by one. */
            type& operator ++() noexcept
            {
                if (this->m_position == this->m_size) return *this;
                ++(this->m_position);
                return *this;
            } // operator ++(...)
        }; // struct matrix_iterator
    } // namespace detail

    /** @brief A rectangular array.
     *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
     */
    template <typename t_data_type, bool t_is_row_major = true>
    struct matrix;

    /** A rectangular array stored in row-major order. */
    template <typename t_data_type>
    using matrix_row_major = matrix<t_data_type, true>;

    /** A rectangular array stored in column-major order. */
    template <typename t_data_type>
    using matrix_column_major = matrix<t_data_type, false>;

    /** @brief A rectangular array.
     *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
     */
    template <typename t_data_type, bool t_is_row_major>
    struct matrix
    {
        using type = matrix<t_data_type, t_is_row_major>;
        static constexpr bool is_row_major = t_is_row_major;
        static constexpr bool is_column_major = !t_is_row_major;

        using data_type = t_data_type;
        using value_type = t_data_type;
        using iterator_type = detail::matrix_iterator<data_type, data_type*>;
        using const_iterator_type = detail::matrix_iterator<data_type, const data_type*>;

        template <typename, bool> friend struct matrix;

    private:
        using arrangement_type = typename detail::matrix_arrangement<t_is_row_major>;
                
        template <typename t_other_data_type>
        using other_t = matrix<t_other_data_type, t_is_row_major>;

        data_type m_invalid = { };
        std::size_t m_height; // Height of the matrix.
        std::size_t m_width; // Width of the matrix.
        std::size_t m_size; // Number of elements in the matrix.
        data_type* m_data_pointer = nullptr; // Pointer to matrix data in memory (heap).

        /** @brief Copies raw data from \p data_pointer to this matrix.
         *  @remark Does not perform size-related checks.
         */
        void unchecked_copy_from(const data_type* data_pointer, const std::string& caller_function_name, std::size_t line_number) noexcept
        {
            if (this->m_size == 0) 
            {
                if (data_pointer == nullptr) return; // Both the matrix and data_pointer are empty.
                quiet_error::instance().push(not_an_error::all_good, severity_level::not_at_all, "Trying to copy to an empty matrix.", caller_function_name, line_number);
                return;
            }
            if (data_pointer == nullptr)
            {
                quiet_error::instance().push(not_an_error::invalid_argument, severity_level::major, "Invalid data pointer.", caller_function_name, line_number);
                return;
            }
            std::memcpy(this->m_data_pointer, data_pointer, this->m_size * sizeof(data_type));
        }

        /** @brief Checks if the other matrix is of the same size, pushing \c quiet_error if not. */
        template <typename t_other_data_type>
        bool is_same_size_ensure(const other_t<t_other_data_type>& other, const std::string& caller_function_name, std::size_t line_number) noexcept
        {
            bool result = true;
            quiet_error& err = quiet_error::instance();
            if (this->m_height != other.m_height)
            {
                result = false;
                err.push(not_an_error::logic_error, severity_level::major, "Matrices must have same height.", caller_function_name, line_number);
            }
            if (this->m_width != other.m_width)
            {
                result = false;
                err.push(not_an_error::logic_error, severity_level::major, "Matrices must have same width.", caller_function_name, line_number);
            }
            return result;
        }

        /** @brief Checks if the index is within this matris, pushing \c quiet_error if not. */
        bool is_index_good_ensure(std::size_t row_index, std::size_t column_index, const std::string& caller_function_name, std::size_t line_number) const noexcept
        {
            bool result = true;
            quiet_error& err = quiet_error::instance();
            if (row_index >= this->m_height)
            {
                result = false;
                err.push(not_an_error::out_of_range, severity_level::fatal, "<row_index> must be smaller than the height of the matrix.", caller_function_name, line_number);
            }
            if (column_index >= this->m_width)
            {
                result = false;
                err.push(not_an_error::out_of_range, severity_level::fatal, "<column_index> must be smaller than the width of the matrix.", caller_function_name, line_number);
            }
            return result;
        }

    public:
        /** Creates an empty matrix. */
        matrix() noexcept
            : m_height(0), m_width(0), m_size(0)
        {
        } // matrix(...)

        /** @brief Creates an un-initialized matrix of a given size.
         *  @remark The memory allocated for data storage is zeroed out; no constructors are called for underlying data elements.
         */
        matrix(std::size_t height, std::size_t width) noexcept
            : m_height(height), m_width(width), m_size(height * width)
        {
            if (this->m_size != 0) this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
            if (this->m_data_pointer == nullptr)
            {
                this->m_height = 0;
                this->m_width = 0;
                this->m_size = 0;
            }
            this->wipe();
        } // matrix(...)

        /** Creates a matrix as a copy. */
        matrix(const type& other) noexcept
            : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size)
        {
            if (this->m_size != 0) this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
            if (this->m_data_pointer == nullptr)
            {
                this->m_height = 0;
                this->m_width = 0;
                this->m_size = 0;
            }
            this->unchecked_copy_from(other.m_data_pointer, __FUNCTION__, __LINE__);
        } // matrix(...)

        /** Creates a matrix by stealing from \p other. */
        matrix(type&& other) noexcept
            : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size)
        {
            this->m_data_pointer = other.m_data_pointer; // Steal.
            other.m_data_pointer = nullptr; // Clean up.

            other.m_height = 0;
            other.m_width = 0;
            other.m_size = 0;
        } // matrix(...)

        /** Creates a matrix with all entries set to \p value. */
        matrix(std::size_t height, std::size_t width, const data_type& value) noexcept
            : matrix(height, width)
        {
            this->fill(value);
        } // matrix(...)

        /** Creates a matrix as a copy. */
        matrix(std::size_t height, std::size_t width, const data_type* data_pointer) noexcept
            : matrix(height, width)
        {
            this->unchecked_copy_from(data_pointer, __FUNCTION__, __LINE__);
        } // matrix(...)

        /** Creates a matrix from a vector. */
        matrix(const std::vector<data_type>& value) noexcept
            : matrix(value.size(), 1)
        {
            this->unchecked_copy_from(value.data(), __FUNCTION__, __LINE__);
        } // matrix(...)

        /** Copies a matrix. */
        type& operator =(const type& other) noexcept
        {
            if (this != &other)
            {
                // Resize check.
                if (this->m_size != other.m_size)
                {
                    this->m_height = other.m_height;
                    this->m_width = other.m_width;
                    this->m_size = other.m_size;

                    // Free up existing memory.
                    delete this->m_data_pointer;
                    this->m_data_pointer = nullptr;
                            
                    // Allocate new memory.
                    if (this->m_size != 0) this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
                    if (this->m_data_pointer == nullptr)
                    {
                        this->m_height = 0;
                        this->m_width = 0;
                        this->m_size = 0;
                    }
                }
                this->unchecked_copy_from(other.m_data_pointer, __FUNCTION__, __LINE__);
            }
            return *this;
        } // operator =(...)

        /** Copies a matrix by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            delete this->m_data_pointer; // Clean up.
            this->m_data_pointer = other.m_data_pointer; // Steal.
            other.m_data_pointer = nullptr;

            this->m_height = other.m_height;
            this->m_width = other.m_width;
            this->m_size = other.m_size;

            other.m_height = 0;
            other.m_width = 0;
            other.m_size = 0;
                    
            return *this;
        } // operator =(...)

        ~matrix() noexcept
        {
            delete this->m_data_pointer;
            this->m_data_pointer = nullptr;

            this->m_height = 0;
            this->m_width = 0;
            this->m_size = 0;
        } // ~matrix(...)

        /** @brief Overwrites allocated memory with zeros.
         *  @warning No destructors will be called on existing elements.
         */
        void wipe() noexcept
        {
            if (this->m_data_pointer == nullptr) return;
            std::memset(this->m_data_pointer, 0, this->m_size * sizeof(data_type));
        } // wipe(...)

        /** Fills matrix with \p value. */
        void fill(const data_type& value) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] = value;
        } // fill(...)

        /** Transforms every element of the matrix by applying \p action to it. */
        template <typename t_action_type>
        void transform(t_action_type&& action) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] = action(this->m_data_pointer[k]);
        } // transform(...)

        /** Height of the matrix. */
        std::size_t height() const noexcept { return this->m_height; }

        /** Width of the matrix. */
        std::size_t width() const noexcept { return this->m_width; }
                
        /** Number of elements in the matrix. */
        std::size_t size() const noexcept { return this->m_size; }

        /** Checks if the matrix is empty. */
        bool empty() const noexcept { return this->m_size == 0; }

        /** @brief Re-shape the matrix.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if new size does not match old size.
         *  @remark The behavior of this operation depends on whether this is a row- or column-major matrix.
         */
        void reshape(std::size_t height, std::size_t width) noexcept
        {
            if (height * width != this->m_size)
            {
                quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "New size should match old size.", __FUNCTION__, __LINE__);
                return;
            }
            this->m_height = height;
            this->m_width = width;
        } // reshape(...)
                
        /** Access the first matrix element. */
        const data_type& front() const noexcept { return this->m_size == 0 ? this->m_invalid : this->m_data_pointer[0]; }
                
        /** Access the last matrix element. */
        const data_type& back() const noexcept { return this->m_size == 0 ? this->m_invalid : this->m_data_pointer[this->m_size - 1]; }

        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(std::size_t row_index, std::size_t column_index) const noexcept { return row_index < this->m_height && column_index < this->m_width; }
        
        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(const matrix_index& index) const noexcept { return index.row < this->m_height && index.column < this->m_width; }

        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        data_type& unchecked_at(std::size_t row_index, std::size_t column_index)
        {
            return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
        } // unchecked_at(...)

        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        const data_type& unchecked_at(std::size_t row_index, std::size_t column_index) const
        {
            return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
        } // unchecked_at(...)

        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        data_type& unchecked_at(const matrix_index& index) { return this->unchecked_at(index.row, index.column); }
        
        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        const data_type& unchecked_at(const matrix_index& index) const { return this->unchecked_at(index.row, index.column); }

        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        data_type& operator [](const matrix_index& index) { return this->unchecked_at(index.row, index.column); }
        
        /** @brief Access matrix elements.
         *  @warning Does not perform size-related checks.
         */
        const data_type& operator [](const matrix_index& index) const { return this->unchecked_at(index.row, index.column); }
                
        /** @brief Access matrix elements.
         *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p row_index is out of range.
         *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p column_index is out of range.
         */
        data_type& at(std::size_t row_index, std::size_t column_index) noexcept
        {
            if (!this->is_index_good_ensure(row_index, column_index, __FUNCTION__, __LINE__)) return this->m_invalid;
            return this->unchecked_at(row_index, column_index);
        } // at(...)
                 
        /** @brief Access matrix elements.
         *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p row_index is out of range.
         *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p column_index is out of range.
         */
        const data_type& at(std::size_t row_index, std::size_t column_index) const noexcept
        {
            if (!this->is_index_good_ensure(row_index, column_index, __FUNCTION__, __LINE__)) return this->m_invalid;
            return this->unchecked_at(row_index, column_index);
        } // at(...)

        /** Checks two matrices for equality. */
        bool operator ==(const type& other) const noexcept
        {
            if (this->m_height != other.m_height) return false;
            if (this->m_width != other.m_width) return false;
            if (this->m_data_pointer == nullptr) return (other.m_data_pointer == nullptr);
            if (other.m_data_pointer == nullptr) return (this->m_data_pointer == nullptr);

            for (std::size_t k = 0; k < this->m_size; k++) if (this->m_data_pointer[k] != other.m_data_pointer[k]) return false;

            return true;
        } // operator ==(...)

        /** Checks two matrices for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        /** @brief Adds another matrix to this one.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator +=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] += static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator +=(...)

        /** Adds \p factor to this matrix. */
        template <typename t_scalar_type>
        std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator +=(const t_scalar_type& factor) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] += static_cast<data_type>(factor);
            return *this;
        } // operator +=(...)

        /** @brief Subtracts another matrix from this one.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator -=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] -= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator -=(...)

        /** Subtracts \p factor from this matrix. */
        template <typename t_scalar_type>
        std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator -=(const t_scalar_type& factor) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] -= static_cast<data_type>(factor);
            return *this;
        } // operator -=(...)

        /** @brief Multiples this matrix by another one, elementwise.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator *=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] *= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator *=(...)

        /** Multiples this matrix by \p factor. */
        template <typename t_scalar_type>
        std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator *=(const t_scalar_type& factor) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] *= static_cast<data_type>(factor);
            return *this;
        } // operator *=(...)

        /** @brief Divides this matrix by another one, elementwise.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator /=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] /= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator /=(...)

        /** Divides this matrix by \p factor. */
        template <typename t_scalar_type>
        std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator /=(const t_scalar_type& factor) noexcept
        {
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] /= static_cast<data_type>(factor);
            return *this;
        } // operator /=(...)

        /** @brief Does bitwise "and" of this matrix with another one, elementwise.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator &=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] &= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator &=(...)

        /** @brief Does bitwise "or" of this matrix with another one, elementwise.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator |=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] |= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator |=(...)

        /** @brief Does bitwise "xor" of this matrix with another one, elementwise.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
         */
        template <typename t_other_data_type>
        type& operator ^=(const other_t<t_other_data_type>& other) noexcept
        {
            if (!this->is_same_size_ensure(other, __FUNCTION__, __LINE__)) return *this;
            for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] ^= static_cast<data_type>(other.m_data_pointer[k]);
            return *this;
        } // operator ^=(...)
                
        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators. */
        friend type operator +(type left, const type& right) noexcept { left += right; return left; }
        friend type operator -(type left, const type& right) noexcept { left -= right; return left; }
        friend type operator *(type left, const type& right) noexcept { left *= right; return left; }
        friend type operator /(type left, const type& right) noexcept { left /= right; return left; }
        friend type operator &(type left, const type& right) noexcept { left &= right; return left; }
        friend type operator |(type left, const type& right) noexcept { left |= right; return left; }
        friend type operator ^(type left, const type& right) noexcept { left ^= right; return left; }

        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_data_pointer, this->m_size, 0); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_data_pointer, this->m_size, this->m_size); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_data_pointer, this->m_size, 0); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_data_pointer, this->m_size, this->m_size); }

        iterator_type begin() noexcept { return iterator_type(this->m_data_pointer, this->m_size, 0); }
        iterator_type end() noexcept { return iterator_type(this->m_data_pointer, this->m_size, this->m_size); }
    }; // struct matrix

    /** Specialization for \c matrix when there is no need to store any data---except for dimensions. */
    template <bool t_is_row_major>
    struct matrix<void, t_is_row_major>
    {
        using type = matrix<void, t_is_row_major>;
        static constexpr bool is_row_major = t_is_row_major;
        static constexpr bool is_column_major = !t_is_row_major;

        using data_type = void;
        using value_type = void;

    private:
        std::size_t m_height; // Height of the matrix.
        std::size_t m_width; // Width of the matrix.
        std::size_t m_size; // Number of elements in the matrix.

    public:
        /** Creates an empty matrix. */
        matrix() noexcept
            : m_height(0), m_width(0), m_size(0)
        {
        } // matrix(...)

        /** Creates a matrix of a given size. */
        matrix(std::size_t height, std::size_t width) noexcept
            : m_height(height), m_width(width), m_size(height * width)
        {
        } // matrix(...)

        /** Height of the matrix. */
        std::size_t height() const noexcept { return this->m_height; }

        /** Width of the matrix. */
        std::size_t width() const noexcept { return this->m_width; }
                
        /** Number of elements in the matrix. */
        std::size_t size() const noexcept { return this->m_size; }

        /** Checks if the matrix is empty. */
        bool empty() const noexcept { return this->m_size == 0; }

        /** Checks two matrices for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_height == other.m_height &&
                this->m_width != other.m_width;
        } // operator ==(...)

        /** Checks two matrices for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)
    }; // struct matrix<...>
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
