
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/algebra/matrix.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::int64_t
#include <limits>  // std::numeric_limits

namespace ropufu::aftermath::tests
{
    template <typename t_left_matrix_type, typename t_right_matrix_type>
    long double matrix_distance(const t_left_matrix_type& left, const t_right_matrix_type& right) noexcept
    {
        long double result = std::numeric_limits<long double>::infinity();
        if (left.width() != right.width()) return result;
        if (left.height() != right.height()) return result;

        result = 0;
        for (std::size_t i = 0; i < left.height(); ++i)
        {
            for (std::size_t j = 0; j < left.width(); ++j)
            {
                long double diff = static_cast<long double>(left(i, j)) - static_cast<long double>(right(i, j));
                if (diff < 0) diff = -diff;
                if (diff > result) result = diff;
            } // for (...)
        } // for (...)

        return result;
    } // matrix_distance(...)

    template <typename t_matrix_type>
    t_matrix_type zeros_matrix(std::size_t height, std::size_t width) noexcept
    {
        return t_matrix_type(height, width);
    } // zero_matrix(...)

    template <typename t_matrix_type>
    t_matrix_type ones_matrix(std::size_t height, std::size_t width) noexcept
    {
        return t_matrix_type(height, width, 1);
    } // ones_matrix(...)

    template <typename t_matrix_type>
    t_matrix_type non_negative_matrix_b(std::size_t height, std::size_t width) noexcept
    {
        using scalar_t = typename t_matrix_type::value_type;
        t_matrix_type result { height, width };

        for (std::size_t i = 0; i < height; ++i) 
            for (std::size_t j = 0; j < width; ++j)
                result(i, j) = static_cast<scalar_t>(i + (j % 2));

        return result;
    } // initialize_matrix_one(...)
} // namespace ropufu::aftermath::tests

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES \
    ropufu::aftermath::algebra::rmatrix_t<std::size_t>,        \
    ropufu::aftermath::algebra::rmatrix_t<std::uint16_t>,      \
    ropufu::aftermath::algebra::rmatrix_t<std::int16_t>,       \
    ropufu::aftermath::algebra::rmatrix_t<std::int32_t>,       \
    ropufu::aftermath::algebra::rmatrix_t<std::int64_t>,       \
    ropufu::aftermath::algebra::rmatrix_t<float>,              \
    ropufu::aftermath::algebra::rmatrix_t<double>,             \
    ropufu::aftermath::algebra::cmatrix_t<std::size_t>,        \
    ropufu::aftermath::algebra::cmatrix_t<std::uint16_t>,      \
    ropufu::aftermath::algebra::cmatrix_t<std::int16_t>,       \
    ropufu::aftermath::algebra::cmatrix_t<std::int32_t>,       \
    ropufu::aftermath::algebra::cmatrix_t<std::int64_t>,       \
    ropufu::aftermath::algebra::cmatrix_t<float>,              \
    ropufu::aftermath::algebra::cmatrix_t<double>              \


TEST_CASE_TEMPLATE("testing matrix type-casting", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t zero = ropufu::aftermath::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::aftermath::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::aftermath::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });
    
    using target_type = ropufu::aftermath::algebra::matrix<float, typename tested_t::arrangement_type>;
    target_type zero_cast { zero };
    target_type one_cast { one };
    target_type b_cast { b };
    target_type c_cast { c };

    CHECK(ropufu::aftermath::tests::matrix_distance(zero, zero_cast) == 0);
    CHECK(ropufu::aftermath::tests::matrix_distance(one, one_cast) == 0);
    CHECK(ropufu::aftermath::tests::matrix_distance(b, b_cast) == 0);
    CHECK(ropufu::aftermath::tests::matrix_distance(c, c_cast) == 0);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing matrix arithmetic 1", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t zero = ropufu::aftermath::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::aftermath::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::aftermath::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });
        
    REQUIRE(zero + one == one);
    REQUIRE(zero + b == b);
    REQUIRE(zero + c == c);
    REQUIRE(b + one == c);

    tested_t d = b;
    tested_t e = c * one;
    tested_t f = c / one;
    d *= zero;

    REQUIRE(d == zero);
    REQUIRE(e == c);
    REQUIRE(f == c);
    REQUIRE(e == f);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing matrix arithmetic 2", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t zero = ropufu::aftermath::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::aftermath::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::aftermath::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });

    scalar_t sc_zero = 0;
    scalar_t sc_one = 1;
    scalar_t sc_two = 2; 
        
    REQUIRE(zero + sc_one == one);
    REQUIRE(one + sc_zero == one);

    tested_t bb = b;
    tested_t cc = c;
    bb *= sc_two;
    cc += sc_two;

    for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) REQUIRE(bb(i, j) == b(i, j) * sc_two);
    for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) REQUIRE(cc(i, j) == c(i, j) + sc_two);

    REQUIRE(b + sc_one == c);

    tested_t d = b;
    tested_t e = c * sc_one;
    tested_t f = c / sc_one;
    d *= sc_zero;

    REQUIRE(d == zero);
    REQUIRE(e == c);
    REQUIRE(f == c);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing matrix slicing", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;
    using slice_t = typename tested_t::slice_type;

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t zero = ropufu::aftermath::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::aftermath::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::aftermath::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });

    slice_t b_diagonal = b.diag();
    slice_t c_diagonal = c.diag();

    std::size_t k = 0;
    for (scalar_t& x : b_diagonal)
    {
        REQUIRE(x == b(k, k));
        ++k;
    } // for (...)
    
    k = 0;
    for (scalar_t& x : c_diagonal)
    {
        REQUIRE(x == c(k, k));
        ++k;
    } // for (...)

    if (height > width && height > 0)
    {
        one.row(0) = b_diagonal;
        for (k = 0; k < width; ++k) REQUIRE(one(0, k) == b(k, k));
    } // if (...)
    if (height <= width && width > 0)
    {
        zero.column(0) = c_diagonal;
        for (k = 0; k < height; ++k) REQUIRE(zero(k, 0) == c(k, k));
    } // if (...)
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED
