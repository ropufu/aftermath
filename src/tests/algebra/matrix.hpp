
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/matrix.hpp"
#include "../../ropufu/algebra/matrix_mask.hpp"

#include <algorithm> // std::sort
#include <cstddef>   // std::size_t
#include <cstdint>   // std::int16_t, std::int32_t, std::int64_t
#include <limits>    // std::numeric_limits
#include <memory>    // std::allocator
#include <unordered_set> // std::unordered_set
#include <vector>    // std::vector

// Note: std::int16_t and std::uint16_t are not closed under addition.
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES \
    ropufu::aftermath::algebra::rmatrix_t<std::size_t>,        \
    ropufu::aftermath::algebra::rmatrix_t<std::int32_t>,       \
    ropufu::aftermath::algebra::rmatrix_t<std::int64_t>,       \
    ropufu::aftermath::algebra::rmatrix_t<float>,              \
    ropufu::aftermath::algebra::rmatrix_t<double>,             \
    ropufu::aftermath::algebra::cmatrix_t<std::size_t>,        \
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

    tested_t zero = ropufu::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });
    
    using target_type = ropufu::aftermath::algebra::matrix<float, std::allocator<float>, typename tested_t::arrangement_type>;
    target_type zero_cast = static_cast<target_type>(zero);
    target_type one_cast = static_cast<target_type>(one);
    target_type b_cast = static_cast<target_type>(b);
    target_type c_cast = static_cast<target_type>(c);

    CHECK(ropufu::tests::matrix_distance(zero, zero_cast) == 0);
    CHECK(ropufu::tests::matrix_distance(one, one_cast) == 0);
    CHECK(ropufu::tests::matrix_distance(b, b_cast) == 0);
    CHECK(ropufu::tests::matrix_distance(c, c_cast) == 0);
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

    tested_t zero = ropufu::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::tests::template non_negative_matrix_b<tested_t>(height, width);
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

    tested_t zero = ropufu::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::tests::template non_negative_matrix_b<tested_t>(height, width);
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

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t zero = ropufu::tests::template zeros_matrix<tested_t>(height, width);
    tested_t one = ropufu::tests::template ones_matrix<tested_t>(height, width);
    tested_t b = ropufu::tests::template non_negative_matrix_b<tested_t>(height, width);
    tested_t c = tested_t::generate(height, width,
        [&b] (std::size_t i, std::size_t j) { return static_cast<scalar_t>(b(i, j) + 1); });

    auto b_diagonal = b.diag();
    auto c_diagonal = c.diag();

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

TEST_CASE_TEMPLATE("testing masked slicing", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;
    using mask_t = typename tested_t::mask_type;
    using index_t = typename tested_t::index_type;

    std::size_t height = 0;
    std::size_t width = 0;

    SUBCASE("") { height = 1; width = 1; }
    SUBCASE("") { height = 2; width = 0; }
    SUBCASE("") { height = 5; width = 3; }
    SUBCASE("") { height = 4; width = 7; }

    CAPTURE(height);
    CAPTURE(width);

    tested_t b = ropufu::tests::template non_negative_matrix_b<tested_t>(height, width);

    mask_t all {height, width, true};
    mask_t none {height, width, false};
    mask_t some = b.make_mask(false);

    index_t i1 { 1, 0 };
    index_t i2 { height / 2, width / 3 };
    index_t i3 { height / 4, 1 };
    
    std::unordered_set<index_t> indices {};
    std::vector<scalar_t> reference_values {};
    reference_values.reserve(3);
    if (i1.row < height && i1.column < width) indices.insert(i1);
    if (i2.row < height && i2.column < width) indices.insert(i2);
    if (i3.row < height && i3.column < width) indices.insert(i3);
    for (const index_t& i : indices)
    {
        some[i] = true;
        reference_values.push_back(b[i]);
    } // for (...)
    std::sort(reference_values.begin(), reference_values.end());

    REQUIRE(b[all].size() == b.size());
    REQUIRE(b[none].size() == 0);
    REQUIRE(b[some].size() == reference_values.size());

    std::vector<scalar_t> tested_values {};
    tested_values.reserve(3);
    for (const scalar_t& x : b[some]) tested_values.push_back(x);
    std::sort(tested_values.begin(), tested_values.end());

    for (std::size_t k = 0; k < reference_values.size(); ++k)
        CHECK(tested_values[k] == reference_values[k]);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_MATRIX_HPP_INCLUDED
