
#ifndef ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/matrix.hpp"
#include "../../ropufu/algebra/matrix_mask.hpp"
#include "../../ropufu/algorithm/rook_pivoting.hpp"
#include "../../ropufu/algorithm/lower_upper_decomposition.hpp"

#include <algorithm> // std::sort
#include <cstddef>   // std::size_t
#include <limits>    // std::numeric_limits
#include <memory>    // std::allocator
#include <unordered_set> // std::unordered_set
#include <vector>    // std::vector

#define ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES \
    ropufu::aftermath::algebra::rmatrix_t<float>,              \
    ropufu::aftermath::algebra::rmatrix_t<double>,             \
    ropufu::aftermath::algebra::rmatrix_t<long double>,        \
    ropufu::aftermath::algebra::cmatrix_t<double>              \


TEST_CASE_TEMPLATE("testing LU decomposition roundtrip 1", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

    matrix_type a = {
        {0, 5, 1, 0},
        {0, 1, 5, 4},
        {1, 0, 0, 2}
    };

    pivoting_type pivoting {};
    decomposition_type lu(a, pivoting);

    REQUIRE(lu.upper().upper_triangular());
    REQUIRE(lu.lower_inverse().lower_triangular());

    matrix_type a_roundtrip = matrix_type::matrix_multiply(lu.lower(), lu.upper());
    for (auto it = lu.column_swaps().rbegin(); it != lu.column_swaps().rend(); ++it) a_roundtrip.try_swap_columns(it->first, it->second);
    for (auto it = lu.row_swaps().rbegin(); it != lu.row_swaps().rend(); ++it) a_roundtrip.try_swap_rows(it->first, it->second);

    CHECK(ropufu::tests::matrix_distance(a, a_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing LU decomposition roundtrip 2", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

    matrix_type a = {
        {0, 0, 2},
        {0, 3, 0},
        {4, 0, 0}
    };

    pivoting_type pivoting{};
    decomposition_type lu(a, pivoting);

    REQUIRE(lu.upper().upper_triangular());
    REQUIRE(lu.lower_inverse().lower_triangular());

    matrix_type a_roundtrip = matrix_type::matrix_multiply(lu.lower(), lu.upper());
    for (auto it = lu.column_swaps().rbegin(); it != lu.column_swaps().rend(); ++it) a_roundtrip.try_swap_columns(it->first, it->second);
    for (auto it = lu.row_swaps().rbegin(); it != lu.row_swaps().rend(); ++it) a_roundtrip.try_swap_rows(it->first, it->second);

    CHECK(ropufu::tests::matrix_distance(a, a_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing LU decomposition roundtrip 3", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

    matrix_type a = {
        {0, 0, 0},
        {0, 3, 1},
        {0, 1, 2}
    };

    pivoting_type pivoting{};
    decomposition_type lu(a, pivoting);

    REQUIRE(lu.upper().upper_triangular());
    REQUIRE(lu.lower_inverse().lower_triangular());

    matrix_type a_roundtrip = matrix_type::matrix_multiply(lu.lower(), lu.upper());
    for (auto it = lu.column_swaps().rbegin(); it != lu.column_swaps().rend(); ++it) a_roundtrip.try_swap_columns(it->first, it->second);
    for (auto it = lu.row_swaps().rbegin(); it != lu.row_swaps().rend(); ++it) a_roundtrip.try_swap_rows(it->first, it->second);

    CHECK(ropufu::tests::matrix_distance(a, a_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing LU decomposition solver 2", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

    matrix_type a = {
        {0, 0, 2},
        {0, 3, 0},
        {4, 0, 0}
    };

	matrix_type b = {
		{1, 4},
		{2, 5},
		{3, 6}
	};

    pivoting_type pivoting{};
    decomposition_type lu(a, pivoting);
    
	matrix_type x = lu.solve(b);
	matrix_type b_roundtrip = matrix_type::matrix_multiply(a, x);

    CHECK(ropufu::tests::matrix_distance(b, b_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing LU decomposition solver 3", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

	matrix_type a = {
		{1, -1, 2},
		{3, 3, 0},
		{4, 0, 5}
	};

	matrix_type b = {
		{1, 4},
		{2, 5},
		{3, 6}
	};

    pivoting_type pivoting{};
    decomposition_type lu(a, pivoting);
    
	matrix_type x = lu.solve(b);
	matrix_type b_roundtrip = matrix_type::matrix_multiply(a, x);

    CHECK(ropufu::tests::matrix_distance(b, b_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing LU decomposition solver 4 (degenerate)", matrix_type, ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_FLOAT_TYPES)
{
    using value_type = typename matrix_type::value_type;
    using decomposition_type = ropufu::aftermath::algorithm::lower_upper_decomposition_t<matrix_type>;
    using pivoting_type = ropufu::aftermath::algorithm::rook_pivoting<value_type>;

	matrix_type a = {
		{0, 0, 0},
		{0, 3, 1},
		{0, 1, 2}
	};

	matrix_type b = {
		{0},
		{2},
		{3}
	};

    pivoting_type pivoting{};
    decomposition_type lu(a, pivoting);
    
	matrix_type x = lu.solve(b);
	matrix_type b_roundtrip = matrix_type::matrix_multiply(a, x);

    CHECK(ropufu::tests::matrix_distance(b, b_roundtrip) < 1e-5);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGORITHM_LOWER_UPPER_DECOMPOSITION_HPP_INCLUDED
