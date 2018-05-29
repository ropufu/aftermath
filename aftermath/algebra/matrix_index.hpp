
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED

#include <cstddef>    // std::size_t
#include <functional> // std::hash

namespace ropufu::aftermath::algebra
{
    struct matrix_index
    {
        using type = matrix_index;

        std::size_t row = 0;
        std::size_t column = 0;

        matrix_index() noexcept { }
        matrix_index(std::size_t row, std::size_t column) noexcept : row(row), column(column) { }

        void offset(type shift) noexcept
        {
            this->row += shift.row;
            this->column += shift.column;
        } // offset(...)

        void offset(std::size_t row_shift, std::size_t column_shift) noexcept
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
    template <>
    struct hash<ropufu::aftermath::algebra::matrix_index>
    {
        using argument_type = ropufu::aftermath::algebra::matrix_index;
        using result_type = std::size_t;

        result_type operator()(const argument_type& x) const noexcept { return x.row ^ x.column; }
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INDEX_HPP_INCLUDED
