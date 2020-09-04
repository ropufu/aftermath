
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_PARTITIONED_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_PARTITIONED_VECTOR_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/partitioned_vector.hpp"

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::int64_t
#include <limits>  // std::numeric_limits
#include <list>    // std::list
#include <vector>  // std::vector

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_VECTOR_ARITHMETIC_TYPES \
    ropufu::aftermath::simple_vector<std::size_t>,             \
    ropufu::aftermath::simple_vector<std::uint16_t>,           \
    ropufu::aftermath::simple_vector<std::int16_t>,            \
    ropufu::aftermath::simple_vector<std::int32_t>,            \
    ropufu::aftermath::simple_vector<std::int64_t>,            \
    ropufu::aftermath::simple_vector<float>,                   \
    ropufu::aftermath::simple_vector<double>                   \


TEST_CASE_TEMPLATE("testing partitioned vector", simple_vector_type, ROPUFU_AFTERMATH_TESTS_ALGEBRA_VECTOR_ARITHMETIC_TYPES)
{
    using value_type = typename simple_vector_type::value_type;
    using allocator_type = typename simple_vector_type::allocator_type;
    using partitioned_vector_type = ropufu::aftermath::partitioned_vector<value_type, allocator_type>;

    std::size_t size = 0;

    SUBCASE("") { size = 1; }
    SUBCASE("") { size = 0; }
    SUBCASE("") { size = 3; }
    SUBCASE("") { size = 7; }

    CAPTURE(size);

    simple_vector_type zero {size};
    simple_vector_type one {size};
    simple_vector_type b {size};

    value_type temp = 1729;
    for (value_type& x : one) x = 1;
    for (value_type& x : b) { x = temp / 2; temp = static_cast<value_type>(static_cast<int>(temp * (temp - 1)) % 3203); }
    
    partitioned_vector_type zero_partitioned {zero};
    partitioned_vector_type one_partitioned {one};
    partitioned_vector_type b_partitioned {b};

    CHECK_EQ(zero_partitioned.partition_size(), 1);
    CHECK_EQ(one_partitioned.partition_size(), 1);
    CHECK_EQ(b_partitioned.partition_size(), 1);

    std::size_t sz {};

    sz = 0;
    for ([[maybe_unused]] value_type x : zero_partitioned) ++sz;
    CHECK_EQ(sz, size);

    sz = 0;
    for ([[maybe_unused]] value_type x : one_partitioned) ++sz;
    CHECK_EQ(sz, size);

    sz = 0;
    for ([[maybe_unused]] value_type x : b_partitioned) ++sz;
    CHECK_EQ(sz, size);

    // ~~ Add a trivial partition element ~~
    zero_partitioned.split(size);
    one_partitioned.split(size);
    b_partitioned.split(size);

    CHECK_EQ(zero_partitioned.partition_size(), 2);
    CHECK_EQ(one_partitioned.partition_size(), 2);
    CHECK_EQ(b_partitioned.partition_size(), 2);

    // ~~ Activate the trivial partition element ~~
    zero_partitioned.activate_partition(1);
    one_partitioned.activate_partition(1);
    b_partitioned.activate_partition(1);

    CHECK_EQ(zero_partitioned.begin(), zero_partitioned.end());
    CHECK_EQ(one_partitioned.begin(), one_partitioned.end());
    CHECK_EQ(b_partitioned.begin(), b_partitioned.end());

    // ~~ Activate the other partition element ~~
    zero_partitioned.activate_partition(0);
    one_partitioned.activate_partition(0);
    b_partitioned.activate_partition(0);

    auto zero_it = zero.begin();
    auto one_it = one.begin();
    auto b_it = b.begin();

    for (value_type x : zero_partitioned) CHECK_EQ(x, (*(zero_it++)));
    for (value_type x : one_partitioned) CHECK_EQ(x, (*(one_it++)));
    for (value_type x : b_partitioned) CHECK_EQ(x, (*(b_it++)));

    // ~~ Add a halfway partition element ~~
    std::size_t offset = size / 2;
    zero_partitioned.split(offset);
    one_partitioned.split(offset);
    b_partitioned.split(offset);

    CHECK_EQ(zero_partitioned.partition_size(), 3);
    CHECK_EQ(one_partitioned.partition_size(), 3);
    CHECK_EQ(b_partitioned.partition_size(), 3);

    // ~~ Activate the second-half partition element ~~
    zero_partitioned.activate_partition(1);
    one_partitioned.activate_partition(1);
    b_partitioned.activate_partition(1);

    zero_it = zero.begin() + offset;
    one_it = one.begin() + offset;
    b_it = b.begin() + offset;

    for (value_type x : zero_partitioned) CHECK_EQ(x, (*(zero_it++)));
    for (value_type x : one_partitioned) CHECK_EQ(x, (*(one_it++)));
    for (value_type x : b_partitioned) CHECK_EQ(x, (*(b_it++)));
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_PARTITIONED_VECTOR_HPP_INCLUDED
