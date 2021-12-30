
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_SLIDING_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_SLIDING_ARRAY_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/sliding_array.hpp"
#include "../core.hpp"

#include <array>      // std::array
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <random>     // std::mt19937
#include <stdexcept>  // std::logic_error
#include <utility>    // std::move

#define ROPUFU_AFTERMATH_TESTS_SLIDING_ARRAY_ALL_TYPES     \
    ropufu::aftermath::sliding_array<std::size_t, 0>,      \
    ropufu::aftermath::sliding_array<float, 1>,            \
    ropufu::aftermath::sliding_array<double, 2>,           \
    ropufu::aftermath::sliding_array<long double, 3>,      \
    ropufu::aftermath::sliding_array<char, 4>              \


TEST_CASE_TEMPLATE("testing (randomized) sliding_array", sliding_array_type, ROPUFU_AFTERMATH_TESTS_SLIDING_ARRAY_ALL_TYPES)
{
    using engine_type = std::mt19937;
    using value_type = typename sliding_array_type::value_type;

    engine_type engine {};
    ropufu::tests::seed(engine);

    constexpr std::size_t sample_size = 80;
    std::array<value_type, sample_size> sequence {};
    for (value_type& x : sequence) x = static_cast<value_type>(engine());

    sliding_array_type window {};
    // Initial value.
    for (std::size_t i = 0; i < window.size(); ++i) REQUIRE(window[i] == 0);
    // Transitionary period.
    for (std::size_t i = 0; i < window.size(); ++i)
    {
        window.shift_back(sequence[i]);
        std::size_t index_of_first_observation = window.size() - i - 1;
        for (std::size_t k = 0; k < index_of_first_observation; ++k) REQUIRE(window[k] == 0);
        for (std::size_t k = index_of_first_observation; k < window.size(); ++k)
            REQUIRE(window[k] == sequence[k - index_of_first_observation]);
    } // for (...)
    // Stationary run.
    for (std::size_t i = window.size(); i < sample_size; ++i)
    {
        window.shift_back(sequence[i]);
        for (std::size_t k = 0; k < window.size(); ++k)
            REQUIRE(window[k] == sequence[(k + i + 1) - window.size()]);
    } // for (...)
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing sliding_vector(0)", sliding_array_type, ROPUFU_AFTERMATH_TESTS_SLIDING_ARRAY_ALL_TYPES)
{
    using value_type = typename sliding_array_type::value_type;
    using sliding_vector_type = ropufu::aftermath::sliding_vector<value_type>;

    value_type zero = 0;
    value_type two = 2;

    sliding_vector_type a = sliding_vector_type();
    sliding_vector_type b = sliding_vector_type(1);

    a.shift_back(two);
    b.shift_back(two);

    REQUIRE_EQ(a.size(), 0);
    REQUIRE_EQ(b.size(), 1);
    REQUIRE_EQ(b[0], two);

    a = std::move(b);
    REQUIRE_EQ(b.data(), nullptr);
    REQUIRE_EQ(a.size(), 1);
    REQUIRE_EQ(a[0], two);

    a.shift_back(zero);
    REQUIRE_EQ(a[0], zero);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing sliding_vector(5)", sliding_array_type, ROPUFU_AFTERMATH_TESTS_SLIDING_ARRAY_ALL_TYPES)
{
    using value_type = typename sliding_array_type::value_type;
    using sliding_vector_type = ropufu::aftermath::sliding_vector<value_type>;

    value_type zero = 0;
    value_type one = 1;
    value_type two = 2;
    value_type three = 3;

    std::array<value_type, 5> ref{};
    sliding_vector_type a = sliding_vector_type(5);

    a.shift_back(three);
    ref = {zero, zero, zero, zero, three};
    for (std::size_t i = 0; i < ref.size(); ++i) CHECK_EQ(a[i], ref[i]);

    a.shift_back(one);
    ref = {zero, zero, zero, three, one};
    for (std::size_t i = 0; i < ref.size(); ++i) CHECK_EQ(a[i], ref[i]);

    a.shift_back(zero);
    a.shift_back(two);
    ref = {zero, three, one, zero, two};
    for (std::size_t i = 0; i < ref.size(); ++i) CHECK_EQ(a[i], ref[i]);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_SLIDING_ARRAY_HPP_INCLUDED
