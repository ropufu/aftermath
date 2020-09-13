
#ifndef ROPUFU_AFTERMATH_TESTS_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/matrix.hpp"
#include "../../ropufu/format/cat.hpp"
#include "../../ropufu/format/mat4_header.hpp"
#include "../../ropufu/format/mat4_istream.hpp"
#include "../../ropufu/format/mat4_ostream.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::int64_t
#include <filesystem> // std::filesystem::path
#include <random>  // std::mt19937
#include <string>  // std::string

namespace ropufu::tests
{
    template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
    void randomize_matrix(aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat) noexcept
    {
        std::mt19937 engine {};
        ropufu::tests::seed(engine);

        typename std::mt19937::result_type norm = std::mt19937::max() / 1'000;
        for (t_value_type& x : mat)
        {
            typename std::mt19937::result_type z = engine() / norm;
            z -= 500;
            x = static_cast<t_value_type>(z);
        } // for (...)
    } // randomize_matrix(...)
} // namespace ropufu::tests

#define ROPUFU_AFTERMATH_TESTS_FORMAT_MAT4_STREAM_BASE_TRIPLET_TYPES            \
    ropufu::tests::type_triplet<std::int32_t, std::uint16_t, float>, \
    ropufu::tests::type_triplet<std::int32_t, std::int16_t, double>, \
    ropufu::tests::type_triplet<std::uint16_t, std::int32_t, float>, \
    ropufu::tests::type_triplet<std::uint16_t, std::uint8_t, float>, \
    ropufu::tests::type_triplet<std::int32_t, double, std::int32_t>, \
    ropufu::tests::type_triplet<float, std::int32_t, float>,         \
    ropufu::tests::type_triplet<double, float, float>                \


TEST_CASE_TEMPLATE("testing mat4_stream_base", triplet_t, ROPUFU_AFTERMATH_TESTS_FORMAT_MAT4_STREAM_BASE_TRIPLET_TYPES)
{
    using left_scalar_t = typename triplet_t::left_type;
    using middle_type_scalar_t = typename triplet_t::middle_type;
    using right_type_scalar_t = typename triplet_t::right_type;

    using matrix_a_t = ropufu::aftermath::algebra::cmatrix_t<left_scalar_t>;
    using matrix_b_t = ropufu::aftermath::algebra::rmatrix_t<middle_type_scalar_t>;
    using matrix_c_t = ropufu::aftermath::algebra::rmatrix_t<right_type_scalar_t>;

    matrix_a_t a = matrix_a_t::uninitialized(5, 2);
    matrix_b_t b = matrix_b_t::uninitialized(4, 7);
    matrix_c_t c = matrix_c_t::uninitialized(8, 8);

    ropufu::tests::randomize_matrix(a);
    ropufu::tests::randomize_matrix(b);
    ropufu::tests::randomize_matrix(c);

    std::filesystem::path path = "./temp_1729.mat";
    ropufu::aftermath::format::mat4_istream matin {path};
    ropufu::aftermath::format::mat4_ostream matout {path};

    matout << "Hello" << a;
    matout << "World" << b;
    matout << ropufu::aftermath::format::cat("var", 1729) << c;
    matout << c;
    matout << "AnotherA" << a;

    CHECK(matout.good());

    matrix_a_t a_stored {};
    matrix_b_t b_stored {};
    matrix_c_t c_stored {};
    matrix_a_t e_stored {};

    std::string name_a = "";
    std::string name_b = "";
    std::string name_c = "";

    matin >> name_a;
    matin >> name_b;
    CHECK(name_a == "Hello");
    CHECK(name_b == "Hello");

    matin >> a_stored; // "Hello".
    matin >> name_b >> b_stored; // "World".
    matin >> name_c >> c_stored; // "var1729".
    matin >> name_a; // "".
    matin >> ropufu::aftermath::format::mat4_istream_manip::skip;
    matin >> e_stored; // "AnotherA".
    CHECK(name_b == "World");
    CHECK(name_c == "var1729");
    CHECK(name_a == "");

    CHECK(a == a_stored);
    CHECK(b == b_stored);
    CHECK(c == c_stored);
    CHECK(a == e_stored);

    std::filesystem::remove(path);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED
