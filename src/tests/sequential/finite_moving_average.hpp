
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_FINITE_MOVING_AVERAGE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_FINITE_MOVING_AVERAGE_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/iid_persistent_process.hpp"
#include "../../ropufu/sequential/finite_moving_average.hpp"

#include <cstddef>    // std::size_t
#include <random>     // std::mt19937_64
#include <string>     // std::string

#ifdef ROPUFU_TMP_TEST_TYPES
#undef ROPUFU_TMP_TEST_TYPES
#endif
#define ROPUFU_TMP_TEST_TYPES            \
    ropufu::aftermath::random::binomial_sampler<std::mt19937_64>,    \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937_64>,  \
    ropufu::aftermath::random::uniform_int_sampler<std::mt19937_64>  \


#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing finite_moving_average json", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    using value_type = typename process_type::value_type;
    using container_type = typename process_type::container_type;
    using finite_moving_average_type = ropufu::aftermath::sequential::finite_moving_average<value_type, container_type>;
    
    finite_moving_average_type finite_moving_average_o{};
    finite_moving_average_type finite_moving_average_a{5, {1, 2, 3}};
    finite_moving_average_type finite_moving_average_b{10, {1, 2, 3}};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(finite_moving_average_o, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(finite_moving_average_a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(finite_moving_average_b, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing finite_moving_average accumulation", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    using value_type = typename process_type::value_type;
    using container_type = typename process_type::container_type;
    using finite_moving_average_type = ropufu::aftermath::sequential::finite_moving_average<value_type, container_type>;
    process_type proc{};
    finite_moving_average_type finite_moving_average{5, {1, 2, 3}};

    REQUIRE(proc.try_register_observer(&finite_moving_average));

    constexpr std::size_t count = 100;
    for (std::size_t i = 0; i < count; ++i) proc.next();

    CHECK_EQ(finite_moving_average.count_observations(), count);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_FINITE_MOVING_AVERAGE_HPP_INCLUDED
