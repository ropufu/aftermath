
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_MONTE_CARLO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_MONTE_CARLO_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/random/monte_carlo.hpp"

#include <chrono>    // std::chrono::milliseconds
#include <cstddef>   // std::size_t
#include <stdexcept> // std::logic_error
#include <thread>    // std::this_thread::sleep_for

namespace ropufu::tests
{
    struct simple_simulator
    {
        int operator ()() const
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return 1;
        } // operator ()(...)
    }; // struct simple_simulator

    struct simple_aggregator
    {
        long sum = 0;

        void operator()(int value)
        {
            sum += value;
        } // operator ()(...)

        void operator()(const simple_aggregator& other)
        {
            sum += other.sum;
        } // operator ()(...)
    }; // struct simple_aggregator
} // namespace ropufu::tests

TEST_CASE("testing monte_carlo sync")
{
    using simulator_type = ropufu::tests::simple_simulator;
    using aggregator_type = ropufu::tests::simple_aggregator;

    using mc_type_1 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 1>;
    using mc_type_2 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 2>;
    using mc_type_3 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 3>;

    mc_type_1 mc1{};
    mc_type_2 mc2{};
    mc_type_3 mc3{};

    CHECK_EQ(mc1.execute_sync(8).sum, 8);
    CHECK_EQ(mc2.execute_sync(8).sum, 8);
    CHECK_EQ(mc3.execute_sync(8).sum, 8);

    CHECK_EQ(mc1.execute_sync(1).sum, 1);
    CHECK_EQ(mc2.execute_sync(1).sum, 1);
    CHECK_EQ(mc3.execute_sync(1).sum, 1);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE("testing monte_carlo async")
{
    using simulator_type = ropufu::tests::simple_simulator;
    using aggregator_type = ropufu::tests::simple_aggregator;

    using mc_type_1 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 1>;
    using mc_type_2 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 2>;
    using mc_type_3 = ropufu::aftermath::random::monte_carlo<simulator_type, aggregator_type, 3>;

    mc_type_1 mc1{};
    mc_type_2 mc2{};
    mc_type_3 mc3{};

    auto& task1_interrupted = mc1.begin_async(100);
    auto& task2 = mc2.begin_async(8);
    auto& task3 = mc3.begin_async(8);

    CHECK_THROWS_AS(mc1.execute_sync(1), std::logic_error);
    CHECK_THROWS_AS(mc2.execute_sync(1), std::logic_error);
    CHECK_THROWS_AS(mc3.execute_sync(1), std::logic_error);

    task1_interrupted.request_stop();
    CHECK_NE(task1_interrupted.wait().sum, 100);

    CHECK_EQ(task2.wait().sum, 8);
    CHECK_EQ(task3.wait().sum, 8);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_MONTE_CARLO_HPP_INCLUDED
