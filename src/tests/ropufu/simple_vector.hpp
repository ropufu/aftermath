
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_SIMPLE_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_SIMPLE_VECTOR_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/simple_vector.hpp"

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::int64_t
#include <limits>  // std::numeric_limits
#include <list>    // std::list
#include <vector>  // std::vector

namespace ropufu::tests
{
    template <typename t_left_vector_type, typename t_right_vector_type>
    long double vector_distance(const t_left_vector_type& left, const t_right_vector_type& right) noexcept
    {
        long double result = std::numeric_limits<long double>::infinity();
        if (left.size() != right.size()) return result;

        result = 0;
        for (std::size_t i = 0; i < left.size(); ++i)
        {
            long double diff = static_cast<long double>(left.at(i)) - static_cast<long double>(right.at(i));
            if (diff < 0) diff = -diff;
            if (diff > result) result = diff;
        } // for (...)

        return result;
    } // vector_distance(...)

    template <typename t_vector_type>
    t_vector_type zeros_vector(std::size_t size) noexcept
    {
        return t_vector_type(size);
    } // zero_vector(...)

    template <typename t_vector_type>
    t_vector_type ones_vector(std::size_t size) noexcept
    {
        return t_vector_type(size, 1);
    } // ones_vector(...)

    template <typename t_vector_type>
    t_vector_type non_negative_vector_b(std::size_t size) noexcept
    {
        using scalar_t = typename t_vector_type::value_type;
        t_vector_type result { size };

        for (std::size_t i = 0; i < size; ++i)
            result.at(i) = static_cast<scalar_t>(i + (i * i) % 3);

        return result;
    } // initialize_vector_one(...)
} // namespace ropufu::tests

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_VECTOR_ARITHMETIC_TYPES \
    ropufu::aftermath::simple_vector<std::size_t>,             \
    ropufu::aftermath::simple_vector<std::uint16_t>,           \
    ropufu::aftermath::simple_vector<std::int16_t>,            \
    ropufu::aftermath::simple_vector<std::int32_t>,            \
    ropufu::aftermath::simple_vector<std::int64_t>,            \
    ropufu::aftermath::simple_vector<float>,                   \
    ropufu::aftermath::simple_vector<double>                   \


TEST_CASE_TEMPLATE("testing vector constructors", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_VECTOR_ARITHMETIC_TYPES)
{
    using scalar_t = typename tested_t::value_type;
    constexpr std::size_t count = 5;

    std::array<scalar_t, count> a { 1, 2, 5, 0, 7 };
    std::vector<scalar_t> b { a.begin(), a.end() };
    std::list<scalar_t> c { a.begin(), a.end() };

    tested_t vector_a { a };
    tested_t vector_b { b };
    tested_t vector_c { c };
    
    auto a_it = a.begin();
    auto b_it = b.begin();
    auto c_it = c.begin();

    for (std::size_t i = 0; i < count; ++i)
    {
        CHECK((*a_it) == a[i]);
        CHECK((*b_it) == a[i]);
        CHECK((*c_it) == a[i]);

        ++a_it;
        ++b_it;
        ++c_it;
    } // for (...)
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing vector type-casting", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_VECTOR_ARITHMETIC_TYPES)
{
    std::size_t size = 0;

    SUBCASE("") { size = 1; }
    SUBCASE("") { size = 0; }
    SUBCASE("") { size = 3; }
    SUBCASE("") { size = 7; }

    CAPTURE(size);

    tested_t zero = ropufu::tests::template zeros_vector<tested_t>(size);
    tested_t one = ropufu::tests::template ones_vector<tested_t>(size);
    tested_t b = ropufu::tests::template non_negative_vector_b<tested_t>(size);
    
    using target_type = ropufu::aftermath::simple_vector<float>;
    target_type zero_cast = static_cast<target_type>(zero);
    target_type one_cast = static_cast<target_type>(one);
    target_type b_cast = static_cast<target_type>(b);

    CHECK(ropufu::tests::vector_distance(zero, zero_cast) == 0);
    CHECK(ropufu::tests::vector_distance(one, one_cast) == 0);
    CHECK(ropufu::tests::vector_distance(b, b_cast) == 0);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_SIMPLE_VECTOR_HPP_INCLUDED
