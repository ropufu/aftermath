
#ifndef ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/elementwise.hpp"

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <cstdint> // std::int16_t, std::int32_t, std::int64_t
#include <list>    // std::list
#include <utility> // std::pair
#include <vector>  // std::vector

namespace ropufu::aftermath::tests
{
    template <typename t_left_container_type, typename t_right_container_type>
    static bool try_sync_initialize_containers(t_left_container_type& left, t_right_container_type& right) noexcept
    {
        std::size_t min_size = left.size();
        std::size_t max_size = right.size();

        if (min_size > max_size)
        {
            min_size = right.size();
            max_size = left.size();
        } // if (...)
        
        if (min_size != max_size && min_size > 0) return false; // Mismatched non-zero sizes.
        if (max_size == 0) max_size = 13;

        if (!try_initialize_container(left, max_size)) return false;
        if (!try_initialize_container(right, max_size)) return false;

        return true;
    } // try_sync_initialize_containers(...)
} // namespace ropufu::aftermath::tests

#define ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_ALL_TYPES            \
    std::pair<std::vector<std::size_t>, std::array<std::int32_t, 5>>,   \
    std::pair<std::array<std::size_t, 7>, std::vector<std::int32_t>>,   \
    std::pair<std::array<std::size_t, 5>, std::list<std::int32_t>>,     \
    std::pair<std::array<std::size_t, 8>, std::array<std::int32_t, 8>>, \
    std::pair<std::list<std::size_t>, std::vector<std::int32_t>>        \


TEST_CASE("testing elementwise permutations")
{
    for (std::size_t n : std::vector({1, 2, 3, 5, 8, 13, 21}))
    {
        std::vector<std::size_t> identity = ropufu::aftermath::algebra::identity_permutation(n);
        for (std::size_t i = 0; i < n; ++i) REQUIRE(identity[i] == i);
    } // for (...)

    std::vector<char> letters({'a', 'b', 'e', 'c', 'u', 'v', 'y', 'z', 'w', 'x'});
    std::vector<char> expected_sorted_z({'z', 'a', 'b', 'c', 'e', 'u', 'v', 'w', 'x', 'y'});
    std::vector<char> expected_sorted_asc({'a', 'b', 'c', 'e', 'u', 'v', 'w', 'x', 'y', 'z'});
    std::vector<char> expected_sorted_desc({'z', 'y', 'x', 'w', 'v', 'u', 'e', 'c', 'b', 'a'});
    REQUIRE(letters.size() == expected_sorted_z.size());
    REQUIRE(letters.size() == expected_sorted_asc.size());
    REQUIRE(letters.size() == expected_sorted_desc.size());

    std::vector<std::size_t> perm_z = ropufu::aftermath::algebra::permutation(letters,
        [] (char one, char other) { if (one == 'z') return true; if (other == 'z') return false; return one < other; });
    std::vector<std::size_t> perm_asc = ropufu::aftermath::algebra::ascending_permutation(letters);
    std::vector<std::size_t> perm_desc = ropufu::aftermath::algebra::descending_permutation(letters);

    for (std::size_t i = 0; i < letters.size(); ++i)
    {
        REQUIRE(letters[perm_z[i]] == expected_sorted_z[i]);
        REQUIRE(letters[perm_asc[i]] == expected_sorted_asc[i]);
        REQUIRE(letters[perm_desc[i]] == expected_sorted_desc[i]);
    } // for (...)
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing elementwise arithmetic assignment", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_ALL_TYPES)
{
    using first_type = typename tested_t::first_type;
    using second_type = typename tested_t::second_type;

    using left_value_type = typename first_type::value_type;
    using right_value_type = typename second_type::value_type;

    first_type left {};
    second_type right {};
    bool is_good = ropufu::aftermath::tests::try_sync_initialize_containers(left, right);
    REQUIRE(is_good);

    std::size_t n = left.size();
    REQUIRE(right.size() == n);
    CAPTURE(n);

    std::vector<left_value_type> left_vec { left.begin(), left.end() };
    std::vector<right_value_type> right_vec { right.begin(), right.end() };

    first_type left_one = left;
    is_good = ropufu::aftermath::algebra::try_subtract_assign(left_one, right);
    REQUIRE(is_good);
    std::vector<left_value_type> left_modified_one { left_one.begin(), left_one.end() };
    for (std::size_t i = 0; i < n; ++i)
    {
        REQUIRE(left_modified_one[i] == left_vec[i] - right_vec[i]);
    } // for (...)

    first_type left_two = left;
    is_good = ropufu::aftermath::algebra::try_add_assign(left_two, right);
    REQUIRE(is_good);
    std::vector<left_value_type> left_modified_two { left_two.begin(), left_two.end() };
    for (std::size_t i = 0; i < n; ++i)
    {
        REQUIRE(left_modified_two[i] == left_vec[i] + right_vec[i]);
    } // for (...)
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing elementwise binary masks", tested_t, ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_ALL_TYPES)
{
    using first_type = typename tested_t::first_type;
    using second_type = typename tested_t::second_type;

    using left_value_type = typename first_type::value_type;
    // using right_value_type = typename second_type::value_type;

    first_type left {};
    second_type right {};
    bool is_good = ropufu::aftermath::tests::try_sync_initialize_containers(left, right);
    REQUIRE(is_good);

    std::size_t n = left.size();
    REQUIRE(right.size() == n);
    CAPTURE(n);

    std::size_t mask_one = 0;
    std::size_t mask_two = 0;
    ropufu::aftermath::algebra::to_binary_mask(left, [] (left_value_type x) { return (x & 1) == 0; }, mask_one);
    ropufu::aftermath::algebra::to_binary_mask(left, [] (left_value_type x) { return (x & 2) == 0; }, mask_two);
    std::size_t mask_three = (mask_one | mask_two);
    CHECK(mask_one != 0);
    CHECK(mask_two != 0);
    CHECK(mask_three != 0);

    left_value_type sum_one = 0;
    left_value_type sum_two = 0;
    left_value_type sum_three = 0;
    ropufu::aftermath::algebra::masked_sum(left, mask_one, sum_one);
    ropufu::aftermath::algebra::masked_sum(left, mask_two, sum_two);
    ropufu::aftermath::algebra::masked_sum(left, mask_three, sum_three);

    left_value_type expected_sum_one = 0;
    left_value_type expected_sum_two = 0;
    left_value_type expected_sum_three = 0;
    for (left_value_type x : left) if ((x & 1) == 0) expected_sum_one += x;
    for (left_value_type x : left) if ((x & 2) == 0) expected_sum_two += x;
    for (left_value_type x : left) if ((x & 1) == 0 || (x & 2) == 0) expected_sum_three += x;
    CHECK(sum_one == expected_sum_one);
    CHECK(sum_two == expected_sum_two);
    CHECK(sum_three == expected_sum_three);

    sum_one = 0;
    sum_two = 0;
    sum_three = 0;
    ropufu::aftermath::algebra::masked_touch(left, mask_one, [&sum_one] (left_value_type x) { sum_one += x; });
    ropufu::aftermath::algebra::masked_touch(left, mask_two, [&sum_two] (left_value_type x) { sum_two += x; });
    ropufu::aftermath::algebra::masked_touch(left, mask_three, [&sum_three] (left_value_type x) { sum_three += x; });
    CHECK(sum_one == expected_sum_one);
    CHECK(sum_two == expected_sum_two);
    CHECK(sum_three == expected_sum_three);

    std::size_t mask_four = 1729;
    ropufu::aftermath::algebra::masked_action(left, mask_one, [] (left_value_type& x) { x |= 1; });
    ropufu::aftermath::algebra::to_binary_mask(left, [] (left_value_type x) { return (x & 1) == 0; }, mask_four);
    CHECK(mask_four == 0);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
