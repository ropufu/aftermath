
#ifndef ROPUFU_AFTERMATH_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED

#include "../algebra/elementwise.hpp"
#include "../algebra/matrix.hpp"
#include "../algebra/matrix_index.hpp"

#include <cmath>     // std::abs
#include <concepts>  // std::floating_point
#include <cstddef>   // size_type
#include <stdexcept> // std::runtime_error
#include <utility>   // std::swap, std::pair
#include <vector>    // std::vector

namespace ropufu::aftermath::algorithm
{
    /** Performs decomposition on an m-by-n matrix A, succh that
     *  P A Q = L U, where:
     *  -- P is an m-by-m row permutation matrix;
     *  -- Q is an n-by-n column permutation matrix;
     *  -- L is an m-by-m lower triangular matrix;
     *  -- U is an m-by-n upper triangular matrix.
     *  Can also be used to solve systems of linear equations.
     *  Note that, P and Q may be written as a product of individual
     *  swaps:
     *  -- P A = ... P_3 P_2 P_1 A;
     *  -- A Q = A Q_1 Q_2 Q_3 ...;
     *  If we were to use P to permute rows, rather than columns---or
     *  Q to permute columns, rather than rows---the corresponding
     *  sequence of swaps would have to be applied in reverse order:
     * -- B P = B ... P_3 P_2 P1;
     * -- Q B = Q_1 Q_2 Q_3 ... B.
     */
    template <std::floating_point t_value_type, typename t_allocator_type, typename t_arrangement_type>
    struct lower_upper_decomposition;

    template <typename t_matrix_type>
        requires std::floating_point<typename t_matrix_type::value_type>
    using lower_upper_decomposition_t = lower_upper_decomposition<typename t_matrix_type::value_type, typename t_matrix_type::allocator_type, typename t_matrix_type::arrangement_type>;

    template <std::floating_point t_value_type, typename t_allocator_type, typename t_arrangement_type, typename t_pivoting_type>
    static auto make_lower_upper_decomposition(const algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat, const t_pivoting_type& pivoting)
        -> lower_upper_decomposition<t_value_type, t_allocator_type, t_arrangement_type>
    {
        return {mat, pivoting};
    } // make_lower_upper_decomposition(...)
    
    template <std::floating_point t_value_type, typename t_allocator_type, typename t_arrangement_type>
    struct lower_upper_decomposition
    {
        using type = lower_upper_decomposition<t_value_type, t_allocator_type, t_arrangement_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;
        using arrangement_type = t_arrangement_type;

        using matrix_type = algebra::matrix<value_type, allocator_type, arrangement_type>;
        using size_type = typename matrix_type::size_type;
        using matrix_index_type = algebra::matrix_index<size_type>;

    private:
        matrix_type m_lower_inverse;
        matrix_type m_upper;
        std::vector<std::pair<size_type, size_type>> m_row_swaps = {};
        std::vector<std::pair<size_type, size_type>> m_column_swaps = {};

        /** Calculates the inverse of an m-by-m lower triangular matrix L. */
        static matrix_type invert_square_lower(const matrix_type& square_lower) noexcept
        {
            matrix_type result(square_lower);
            size_type m = square_lower.height();
            for (size_type k = 0; k < m; ++k)
            {
                value_type x = result(k, k);
                if (x == 0) continue;

                result(k, k) = 1;
                for (size_type i = k + 1; i < m; ++i) result(i, k) = -result(i, k) / x;
            } // for (...)
            return result;
        } // invert_square_lower(...)

        /** Calculates the product L B, where L is an m-by-k lower triangular matrix, and B is a k-by-n matrix. */
        static matrix_type lower_triangular_multiply(const matrix_type& lower, const matrix_type& b) noexcept
        {
            size_type m = lower.height();
            size_type n = b.width();
            size_type k = b.height();

            size_type s = (m < k) ? (m) : (k);

            matrix_type result(m, n);

            for (size_type i = 0; i < s; ++i)
            {
                size_type t = i + 1;
                for (size_type j = 0; j < n; ++j)
                {
                    value_type& x = result(i, j);
                    for (size_type r = 0; r < t; ++r) x += lower(i, r) * b(r, j);
                } // for (...)
            } // for (...)

            for (size_type i = s; i < m; ++i)
            {
                for (size_type j = 0; j < n; ++j)
                {
                    value_type& x = result(i, j);
                    for (size_type r = 0; r < k; ++r) x += lower(i, r) * b(r, j);
                } // for (...)
            } // for (...)

            return result;
        } // lower_triangular_multiply(...)

        /** Solves the system U X = B, where U is an m-by-k upper triangular matrix and B is an m-by-n matrix. */
        static matrix_type upper_triangular_solve(const matrix_type& upper, const matrix_type& b) noexcept
        {
            size_type m = b.height();
            size_type n = b.width();
            size_type k = upper.width();

            size_type s = (m < k) ? (m) : (k);

            matrix_type result(k, n);
            if (s == 0) return result; // This is a trivial matrix.
            
            // Copy the right-hand side into the solution matrix.
            for (size_type i = 0; i < s; ++i) result.row(i) = b.row(i);

            // Start at the bottom and go up.
            for (size_type r = s - 1; r != 0; --r)
            {
                value_type x = upper(r, r);
                if (x == 0) continue;
                for (size_type j = 0; j < n; ++j)
                {
                    value_type& y = result(r, j);
                    // Re-scale current row.
                    y /= x;
                    // Upward elimination.
                    for (size_type i = 0; i < r; ++i)
                        result(i, j) -= upper(i, r) * y;
                } // for (...)
            } // for (...)

            // Top row.
            value_type x = upper(0, 0);
            if (x != 0) for (size_type j = 0; j < n; ++j) result(0, j) /= x;

            return result;
        } // upper_triangular_solve(...)

    public:
        template <typename t_pivoting_type>
        lower_upper_decomposition(const matrix_type& mat, const t_pivoting_type& pivoting)
            : m_lower_inverse(mat.height(), mat.height()), m_upper(mat)
        {
            size_type m = mat.height();
            size_type n = mat.width();
            size_type s = (m > n) ? (n) : (m);
            
            // Start with L as the m-by-m identity matrix.
            for (size_type k = 0; k < m; ++k) this->m_lower_inverse(k, k) = 1;

            // Pre-allocate storage.
            matrix_type current_lower_inverse(m, m);
            matrix_type temp_lower_storage(m, m);
            matrix_type temp_upper_storage(m, n);
            this->m_row_swaps.reserve(s);
            this->m_column_swaps.reserve(s);
            
            // Perform decomposition.
            // Previous step:
            //     U_old = L_old P_old A Q_old.
            // This step:
            //     U_new = L_current P_current U_old Q_current
            //       = L_current P_current L_old P_old A Q_old Q_current
            //       = L_current P_current L_old (P_current)^(-1) P_current P_old A Q_old Q_current
            //       = L_current P_current L_old P_current P_current P_old A Q_old Q_current
            //       = L_new P_new A Q_new,
            // where
            //     P_new = P_current P_old,
            //     Q_new = Q_old Q_current,
            //     L_new = L_current P_current L_old P_current.
            for (size_type r = 0; r < s; ++r)
            {
                matrix_index_type pivot_index = pivoting(this->m_upper, r);
                if (pivot_index.row < r || pivot_index.row > m) throw std::runtime_error("Pivoting error.");
                if (pivot_index.column < r || pivot_index.column > n) throw std::runtime_error("Pivoting error.");

                value_type x = this->m_upper(pivot_index.row, pivot_index.column); // Current pivot.

                // Record permultations.
                if (pivot_index.row != r) this->m_row_swaps.emplace_back(pivot_index.row, r);
                if (pivot_index.column != r) this->m_column_swaps.emplace_back(pivot_index.column, r);

                // Apply permutations to the upper triangular matrix:
                // U <- P_current U_old Q_current.
                this->m_upper.try_swap_rows(pivot_index.row, r);
                this->m_upper.try_swap_columns(pivot_index.column, r);

                // Apply permutations to the lower triangular matrix:
                // L <- P_current L_old P_current.
                this->m_lower_inverse.try_swap_rows(pivot_index.row, r);
                this->m_lower_inverse.try_swap_columns(pivot_index.row, r);

                if (x == 0) continue; // Zero pivot value encountered: cannot perform elimination.
                // Initialize elemination matrix, L_current.
                current_lower_inverse.make_diagonal(1);
                    for (size_type i = r + 1; i < m; ++i)
                        current_lower_inverse(i, r) = -this->m_upper(i, r) / x;

                // Apply elimination to update the upper triangular matrix:
                // U_new = U <- L_current U = L_current P_current U_old Q_current.
                matrix_type::matrix_multiply(&temp_upper_storage, current_lower_inverse, this->m_upper);
                this->m_upper = temp_upper_storage;

                // Compose (accumulate) lower triangular matrices:
                // L_new = L <- L_current L = L_current P_current L_old P_current.
                matrix_type::matrix_multiply(&temp_lower_storage, current_lower_inverse, this->m_lower_inverse);
                this->m_lower_inverse = temp_lower_storage;
            } // for (...)
        } // lower_upper_decomposition(...)

        const matrix_type& lower_inverse() const noexcept { return this->m_lower_inverse; }
        const matrix_type& upper() const noexcept { return this->m_upper; }

        matrix_type lower() const noexcept
        {
            return type::invert_square_lower(this->m_lower_inverse);
        } // lower(...)

        /** Calculates the generalized determinant of the matrix. */
        value_type determinant() const noexcept
        {
            value_type result = 1;
            for (value_type x : this->m_upper.diag()) result *= x;
            size_type parity = (this->m_row_swaps.size() + this->m_column_swaps.size()) & (0x01);
            return (parity == 0) ? (result) : (-result);
        } // determinant(...)

        const std::vector<std::pair<size_type, size_type>>& row_swaps() const noexcept { return this->m_row_swaps; }
        const std::vector<std::pair<size_type, size_type>>& column_swaps() const noexcept { return this->m_column_swaps; }

        /** Solves a linear system A X = B, where:
         *  -- A is the original m-by-n matrix;
         *  -- B is and m-by-k right-hand side;
         *  -- X is an n-by-k solution matrix.
         */
        matrix_type solve(const matrix_type& right_hand_side) const
        {
            // We already know P A Q = L U.
            // -- Solve L Y = P B for Y.
            // -- Solve U Z = Y for Z.
            // Then X = Q Z.
            if (right_hand_side.height() != this->m_upper.height()) throw std::logic_error("Matrices incompatible.");

            matrix_type b(right_hand_side);
            for (const std::pair<size_type, size_type>& pair : this->m_row_swaps)
                b.try_swap_rows(pair.first, pair.second);

            matrix_type y = type::lower_triangular_multiply(this->m_lower_inverse, b);
            matrix_type z = type::upper_triangular_solve(this->m_upper, y);

            for (auto it = this->m_column_swaps.crbegin(); it != this->m_column_swaps.crend(); ++it)
                z.try_swap_rows(it->first, it->second);

            return z;
        } // solve(...)
    }; // struct lower_upper_decomposition
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED
