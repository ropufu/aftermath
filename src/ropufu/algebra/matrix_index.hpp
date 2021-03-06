
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED

#include <cstddef>    // std::size_t
#include <functional> // std::hash

namespace ropufu::aftermath::algebra
{
    /** @brief A compact way to subscript matrices. */
    template <typename t_size_type = std::size_t>
    struct matrix_index
    {
        using type = matrix_index<t_size_type>;
        using size_type = t_size_type;

        size_type row = 0;
        size_type column = 0;

        matrix_index() noexcept { }
        matrix_index(size_type row, size_type column) noexcept : row(row), column(column) { }

        void offset(type shift) noexcept
        {
            this->row += shift.row;
            this->column += shift.column;
        } // offset(...)

        void offset(size_type row_shift, size_type column_shift) noexcept
        {
            this->row += row_shift;
            this->column += column_shift;
        } // offset(...)

        bool operator ==(const type& other) const noexcept { return this->row == other.row && this->column == other.column; }
        bool operator !=(const type& other) const noexcept { return !(this->operator ==(other)); }
    }; // struct matrix_index
} // namespace ropufu::aftermath::algebra

namespace std
{
    template <typename t_size_type>
    struct hash<ropufu::aftermath::algebra::matrix_index<t_size_type>>
    {
        using argument_type = ropufu::aftermath::algebra::matrix_index<t_size_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_size_type> index_hash {};
            return index_hash((x.row << 8) ^ x.column);
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED
