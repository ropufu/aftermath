
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED

#include <cstddef>
#include <cstring>   // For std::memcpy and std::memset.
#include <new>
#include <stdexcept>
#include <utility>   // For std::move.
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

            private:
                typedef typename detail::matrix_arrangement<t_is_row_major> arrangement_type;

                data_type* m_data_pointer = nullptr; // Pointer to matrix data in memory (heap).
                std::size_t m_height; // Height of the matrix.
                std::size_t m_width; // Width of the matrix.
                std::size_t m_size; // Total number of elements in the matrix.

                /** @brief Copies raw data from \p data_pointer to this matrix.
                 *  @exception std::invalid_argument \p data_pointer is null.
                 *  @remark Does not perform size-related checks.
                 */
                void copy_raw(std::size_t height, std::size_t width, const data_type* data_pointer)
                {
                    if (this->m_data_pointer == nullptr) return; // Does nothing if this matrix is empty.
                    if (data_pointer == nullptr) throw new std::invalid_argument("<data_pointer> cannot be nullptr.");
                    std::memcpy(this->m_data_pointer, data_pointer, (height * width) * sizeof(data_type));
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
                    if (height == 0 || width == 0) return;
                    this->m_data_pointer = new (std::nothrow) data_type[this->m_size];
                    if (this->m_data_pointer == nullptr)
                    {
                        this->m_height = 0;
                        this->m_width = 0;
                        this->m_size = 0;
                    }
                }

                /** Creates a matrix with all entries set to \p value. */
                matrix(std::size_t height, std::size_t width, const data_type& value) noexcept
                    : matrix(height, width)
                {
                    this->fill(value);
                }

                /** @brief Creates a matrix as a copy.
                 *  @remark Does not perform size-related checks.
                 */
                matrix(std::size_t height, std::size_t width, const data_type* data_pointer)
                    : matrix(height, width)
                {
                    this->copy_raw(height, width, data_pointer);
                }

                /** Creates a matrix as a copy. */
                matrix(const type& other) noexcept
                    : matrix(other.m_height, other.m_width, other.m_data_pointer)
                {
                }

                /** Creates a matrix by stealing from \p other. */
                matrix(type&& other) noexcept
                {
                    *this = std::move(other);
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

                            delete this->m_data_pointer; // Free up existing memory.
                            this->m_data_pointer = nullptr;
                        }
                        this->copy_raw(this->m_height, this->m_width, other.m_data_pointer);
                    }
                    return *this;
                }

                /** Copies a matrix by stealing from \p other. */
                matrix& operator =(type&& other) noexcept
                {
                    this->steal(other);
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

                /** Copies a matrix by stealing from \p other. */
                void steal(type& other) noexcept
                {
                    if (this != &other)
                    {
                        delete this->m_data_pointer; // Clean up.
                        this->m_data_pointer = other.m_data_pointer;
                        other.m_data_pointer = nullptr;

                        this->m_height = other.m_height;
                        this->m_width = other.m_width;
                        this->m_size = other.m_size;

                        other.m_height = 0;
                        other.m_width = 0;
                        other.m_size = 0;
                    }
                }

                /** Erases current data and changes the size of the matrix. */
                void resize(std::size_t height, std::size_t width)
                {
                    type other(height, width);
                    this->steal(other);
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
                 *  @exception std::logic_error New size does not match old size.
                 *  @remark The behavior of this operation depends on whether this is a row- or column-major matrix.
                 */
                void reshape(std::size_t height, std::size_t width)
                {
                    if (height * width != this->m_size) throw std::logic_error("New size should match old size.");
                    this->m_height = height;
                    this->m_width = width;
                }

                /** @brief Access matrix elements.
                 *  @remark Does not perform size-related checks.
                 */
                data_type& operator ()(std::size_t row_index, std::size_t column_index)
                {
                    return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
                }

                /** @brief Access matrix elements.
                 *  @remark Does not perform size-related checks.
                 */
                const data_type& operator ()(std::size_t row_index, std::size_t column_index) const
                {
                    return this->m_data_pointer[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)];
                }
                
                /** @brief Access matrix elements.
                 *  @exception std::out_of_range \p row_index is out of range.
                 *  @exception std::out_of_range \p column_index is out of range.
                 */
                 data_type& at(std::size_t row_index, std::size_t column_index)
                 {
                     if (row_index >= this->m_height) throw std::out_of_range("<row_index> must be smaller than the height of the matrix.");
                     if (column_index >= this->m_width) throw std::out_of_range("<column_index> must be smaller than the width of the matrix.");
                     return this->operator ()(row_index, column_index);
                 }
                 
                 /** @brief Access matrix elements.
                  *  @exception std::out_of_range \p row_index is out of range.
                  *  @exception std::out_of_range \p column_index is out of range.
                  */
                const data_type& at(std::size_t row_index, std::size_t column_index) const
                {
                    if (row_index >= this->m_height) throw std::out_of_range("<row_index> must be smaller than the height of the matrix.");
                    if (column_index >= this->m_width) throw std::out_of_range("<column_index> must be smaller than the width of the matrix.");
                    return this->operator ()(row_index, column_index);
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
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
