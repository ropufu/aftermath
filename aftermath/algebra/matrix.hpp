
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED

#include "../not_an_error.hpp"

#include <cstddef>
#include <cstring> // For std::memcpy and std::memset.
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ropufu
{
    namespace aftermath
    {
        namespace algebra
        {
            namespace detail
            {
                /** @brief Describes how a matrix is stored in memory.
                 *  @example Consider the following matrix:
                 *      | a b c |
                 *      | d e f |
                 *    In row-major format it will be stored as (a b c d e f).
                 *    In column-major format it will be stored as (a d b e c f). 
                 */
                template <bool t_is_row_major = true>
                struct matrix_arrangement
                {
                    /** Row-major format. */
                    static std::size_t flatten(std::size_t row_index, std::size_t column_index, std::size_t height, std::size_t width) noexcept
                    {
                        return row_index * width + column_index;
                    }
                };

                template <>
                struct matrix_arrangement<false>
                {
                    /** Column-major format. */
                    static std::size_t flatten(std::size_t row_index, std::size_t column_index, std::size_t height, std::size_t width) noexcept
                    {
                        return column_index * height + row_index;
                    }
                };
            }

            /** A rectangular array. */
            template <typename t_data_type, bool t_is_row_major = true>
            struct matrix;

            /** A rectangular array stored in row-major order. */
            template <typename t_data_type>
            using matrix_row_major = matrix<t_data_type, true>;

            /** A rectangular array stored in column-major order. */
            template <typename t_data_type>
            using matrix_column_major = matrix<t_data_type, false>;

            /** A rectangular array. */
            template <typename t_data_type, bool t_is_row_major>
            struct matrix
            {
                typedef matrix<t_data_type, t_is_row_major> type;
                static constexpr bool is_row_major = t_is_row_major;
                static constexpr bool is_column_major = !t_is_row_major;

                typedef t_data_type data_type;
                typedef t_data_type value_type;

                template <typename, bool> friend struct matrix;

            private:
                typedef typename detail::matrix_arrangement<t_is_row_major> arrangement_type;
                
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
                        quiet_error::instance().push(not_an_error::all_good, "Trying to copy to an empty matrix.", caller_function_name, line_number);
                        return;
                    }
                    if (data_pointer == nullptr)
                    {
                        quiet_error::instance().push(not_an_error::invalid_argument, "Invalid data pointer.", caller_function_name, line_number);
                        return;
                    }
                    std::memcpy(this->m_data_pointer, data_pointer, this->m_size * sizeof(data_type));
                }

            public:
                /** Creates an empty matrix. */
                matrix() noexcept
                    : m_height(0), m_width(0), m_size(0)
                {
                }

                /** Creates a matrix of a given size. */
                matrix(std::size_t height, std::size_t width) noexcept
                    : m_height(height), m_width(width), m_size(height * width)
                {
                    this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
                    if (this->m_data_pointer == nullptr)
                    {
                        this->m_height = 0;
                        this->m_width = 0;
                        this->m_size = 0;
                    }
                    this->erase();
                }

                /** Creates a matrix as a copy. */
                matrix(const type& other) noexcept
                    : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size)
                {
                    this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
                    if (this->m_data_pointer == nullptr)
                    {
                        this->m_height = 0;
                        this->m_width = 0;
                        this->m_size = 0;
                    }
                    this->unchecked_copy_from(other.m_data_pointer, __FUNCTION__, __LINE__);
                }

                /** Creates a matrix by stealing from \p other. */
                matrix(type&& other) noexcept
                    : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size)
                {
                    this->m_data_pointer = other.m_data_pointer; // Steal.
                    other.m_data_pointer = nullptr; // Clean up.

                    other.m_height = 0;
                    other.m_width = 0;
                    other.m_size = 0;
                }

                /** Creates a matrix with all entries set to \p value. */
                matrix(std::size_t height, std::size_t width, const data_type& value) noexcept
                    : matrix(height, width)
                {
                    this->fill(value);
                }

                /** Creates a matrix as a copy. */
                matrix(std::size_t height, std::size_t width, const data_type* data_pointer) noexcept
                    : matrix(height, width)
                {
                    this->unchecked_copy_from(data_pointer, __FUNCTION__, __LINE__);
                }

                /** Creates a matrix from a vector. */
                matrix(const std::vector<data_type>& value) noexcept
                    : matrix(value.size(), 1)
                {
                    this->unchecked_copy_from(value.data(), __FUNCTION__, __LINE__);
                }

                /** Copies a matrix. */
                matrix& operator =(const type& other) noexcept
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
                            this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
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
                }

                /** Copies a matrix by stealing from \p other. */
                matrix& operator =(type&& other) noexcept
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
                }

                ~matrix() noexcept
                {
                    delete this->m_data_pointer;
                    this->m_data_pointer = nullptr;

                    this->m_height = 0;
                    this->m_width = 0;
                    this->m_size = 0;
                }

                /** Fills matrix with zeros. */
                void erase() noexcept
                {
                    if (this->m_data_pointer == nullptr) return;
                    std::memset(this->m_data_pointer, 0, this->m_size * sizeof(data_type));
                }

                /** Fills matrix with \p value. */
                void fill(const data_type& value) noexcept
                {
                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] = value;
                }

                /** Height of the matrix. */
                std::size_t height() const noexcept { return this->m_height; }

                /** Width of the matrix. */
                std::size_t width() const noexcept { return this->m_width; }
                
                /** Number of elements in the matrix. */
                std::size_t size() const noexcept { return this->m_size; }

                /** @brief Re-shape the matrix.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if new size does not match old size.
                 *  @remark The behavior of this operation depends on whether this is a row- or column-major matrix.
                 */
                void reshape(std::size_t height, std::size_t width) noexcept
                {
                    if (height * width != this->m_size)
                    {
                        quiet_error::instance().push(not_an_error::logic_error, "New size should match old size.", __FUNCTION__, __LINE__);
                        return;
                    }
                    this->m_height = height;
                    this->m_width = width;
                }
                
                /** Access the first matrix element. */
                const data_type& front() const noexcept { return this->m_size == 0 ? this->m_invalid : this->m_data_pointer[0]; }
                
                /** Access the last matrix element. */
                const data_type& back() const noexcept { return this->m_size == 0 ? this->m_invalid : this->m_data_pointer[this->m_size - 1]; }

                /** @brief Access matrix elements.
                 *  @warning Does not perform size-related checks.
                 */
                data_type& unchecked_at(std::size_t row_index, std::size_t column_index) noexcept
                {
                    return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
                }

                /** @brief Access matrix elements.
                 *  @warning Does not perform size-related checks.
                 */
                const data_type& unchecked_at(std::size_t row_index, std::size_t column_index) const noexcept
                {
                    return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
                }
                
                /** @brief Access matrix elements.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p row_index is out of range.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p column_index is out of range.
                 */
                data_type& at(std::size_t row_index, std::size_t column_index) noexcept
                {
                    if (row_index >= this->m_height) quiet_error::instance().push(not_an_error::out_of_range, "<row_index> must be smaller than the height of the matrix.", __FUNCTION__, __LINE__);
                    if (column_index >= this->m_width) quiet_error::instance().push(not_an_error::out_of_range, "<column_index> must be smaller than the width of the matrix.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return this->m_invalid;
                    return this->unchecked_at(row_index, column_index);
                }
                 
                 /** @brief Access matrix elements.
                  *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p row_index is out of range.
                  *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p column_index is out of range.
                  */
                const data_type& at(std::size_t row_index, std::size_t column_index) const noexcept
                {
                    if (row_index >= this->m_height) quiet_error::instance().push(not_an_error::out_of_range, "<row_index> must be smaller than the height of the matrix.", __FUNCTION__, __LINE__);
                    if (column_index >= this->m_width) quiet_error::instance().push(not_an_error::out_of_range, "<column_index> must be smaller than the width of the matrix.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return this->m_invalid;
                    return this->unchecked_at(row_index, column_index);
                }

                /** Checks two matrices for equality. */
                bool operator ==(const type& other) const noexcept
                {
                    if (this->m_height != other.m_height) return false;
                    if (this->m_width != other.m_width) return false;
                    if (this->m_data_pointer == nullptr) return (other.m_data_pointer == nullptr);
                    if (other.m_data_pointer == nullptr) return (this->m_data_pointer == nullptr);

                    for (std::size_t k = 0; k < this->m_size; k++) if (this->m_data_pointer[k] != other.m_data_pointer[k]) return false;

                    return true;
                }

                /** Checks two matrices for inequality. */
                bool operator !=(const type& other) const noexcept
                {
                    return !(this->operator ==(other));
                }

                /** @brief Adds another matrix to this one.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator +=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] += static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** Adds \p factor to this matrix. */
                template <typename t_scalar_type>
                std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator +=(const t_scalar_type& factor) noexcept
                {
                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] += static_cast<data_type>(factor);
                    return *this;
                }

                /** @brief Subtracts another matrix from this one.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator -=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] -= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** Subtracts \p factor from this matrix. */
                template <typename t_scalar_type>
                std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator -=(const t_scalar_type& factor) noexcept
                {
                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] -= static_cast<data_type>(factor);
                    return *this;
                }

                /** @brief Multiples this matrix by another one, elementwise.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator *=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] *= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** Multiples this matrix by \p factor. */
                template <typename t_scalar_type>
                std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator *=(const t_scalar_type& factor) noexcept
                {
                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] *= static_cast<data_type>(factor);
                    return *this;
                }

                /** @brief Divides this matrix by another one, elementwise.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator /=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] /= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** Divides this matrix by \p factor. */
                template <typename t_scalar_type>
                std::enable_if_t<std::is_arithmetic<t_scalar_type>::value, type&> operator /=(const t_scalar_type& factor) noexcept
                {
                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] /= static_cast<data_type>(factor);
                    return *this;
                }

                /** @brief Does bitwise "and" of this matrix with another one, elementwise.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator &=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] &= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** @brief Does bitwise "or" of this matrix with another one, elementwise.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator |=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] |= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }

                /** @brief Does bitwise "xor" of this matrix with another one, elementwise.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the matrices are not of the same shape.
                 */
                template <typename t_other_data_type>
                type& operator ^=(const other_t<t_other_data_type>& other) noexcept
                {
                    if (this->m_height != other.m_height) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same height.", __FUNCTION__, __LINE__);
                    if (this->m_width != other.m_width) quiet_error::instance().push(not_an_error::logic_error, "Matrices must have same width.", __FUNCTION__, __LINE__);
                    if (!quiet_error::instance().good()) return *this;

                    for (std::size_t k = 0; k < this->m_size; k++) this->m_data_pointer[k] ^= static_cast<data_type>(other.m_data_pointer[k]);
                    return *this;
                }
                
                /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators */
                friend type operator +(type left, const type& right) { left += right; return left; }
                friend type operator -(type left, const type& right) { left -= right; return left; }
                friend type operator *(type left, const type& right) { left *= right; return left; }
                friend type operator /(type left, const type& right) { left /= right; return left; }
                friend type operator &(type left, const type& right) { left &= right; return left; }
                friend type operator |(type left, const type& right) { left |= right; return left; }
                friend type operator ^(type left, const type& right) { left ^= right; return left; }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
