
#include "../ropufu/probability.hpp"
#include "../ropufu/random.hpp"

#include "definitive_tests.hpp"
#include "random_examples.hpp"
#include "random_test.hpp"
#include "enum_array_test.hpp"

#include "test_algebra.hpp"

#include <exception> // std::exception

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream> // For std::cout etc.
#include <system_error> // std::error_code, std::errc
#include <utility>  // For std::forward.

template <typename t_test_type>
void run_definitive_test(const std::string& name, t_test_type&& test)
{
    bool result = test();
    std::cout << "Test " << (result ? "passed" : "failed") << ": " << name << "." << std::endl;
}

template <typename t_test_type>
void run_error_test(const std::string& name, t_test_type&& test)
{
    double error = test();
    bool result = (error < 0.05);
    std::cout << "Test " << (result ? "passed" : "failed") << " at " << (error * 100) << "%: " << name << "." << std::endl;
}

template <typename t_time_type, typename t_test_type>
void run_benchmark(t_time_type zero, const std::string& name, t_test_type&& test)
{
    t_time_type time_tested = zero;
    t_time_type time_builtin = zero;
    test(time_tested, time_builtin);
    std::cout << "Elapsed time " << time_tested << " vs. " << time_builtin << " seconds (tested vs. built-in) in " << name << "." << std::endl;
}

template <typename t_result_type, typename t_engine_type>
void not_main()
{
    using normal_distribution = ropufu::aftermath::probability::normal_distribution<t_result_type>;
    using lognormal_distribution = ropufu::aftermath::probability::lognormal_distribution<t_result_type>;

    const t_result_type zero = static_cast<t_result_type>(0);
    const t_result_type one = static_cast<t_result_type>(1);

    std::error_code ec {};
    ropufu::test_aftermath::test_random<lognormal_distribution, t_engine_type> lognormal_tester(zero, one, ec);
    if (ec) std::cout << "Test initialization failed." << std::endl;
    ropufu::test_aftermath::test_random<normal_distribution, t_engine_type> normal_tester(zero, one, ec);
    if (ec) std::cout << "Test initialization failed." << std::endl;
    ropufu::test_aftermath::binomial_benchmark<t_engine_type, t_result_type> binomial_bench(10, 11, static_cast<t_result_type>(0.8));


    // ~~ Randomized tests ~~
    run_error_test("lognormal tail probability", [&]() { return lognormal_tester.error_in_tail(10'000'000, static_cast<t_result_type>(8.5)); });
    run_error_test("gaussian tail probability z > 1.8", [&]() { return normal_tester.error_in_tail(10'000'000, static_cast<t_result_type>(1.8)); });
    run_error_test("gaussian tail probability z > 2.5", [&]() { return normal_tester.error_in_tail(10'000'000, static_cast<t_result_type>(2.5)); });
    run_error_test("gaussian tail probability z > 3.0", [&]() { return normal_tester.error_in_tail(10'000'000, static_cast<t_result_type>(3.0)); });
    run_error_test("gaussian CUSUM", [&]() { return normal_tester.error_in_cusum(10'000, static_cast<t_result_type>(std::log(159.36))); });

    // ~~ Benchmarks ~~
    run_benchmark(zero, "lognormal tail probability", [&](t_result_type& s1, t_result_type& s2) { lognormal_tester.benchmark_tail(100'000'000, static_cast<t_result_type>(8.5), s1, s2); });
    run_benchmark(zero, "gaussian tail probability", [&](t_result_type& s1, t_result_type& s2) { normal_tester.benchmark_tail(100'000'000, static_cast<t_result_type>(3.0), s1, s2); });
    run_benchmark(zero, "gaussian CUSUM", [&](t_result_type& s1, t_result_type& s2) { normal_tester.benchmark_cusum(100'000, static_cast<t_result_type>(std::log(159.36)), s1, s2); });
    run_benchmark(zero, "compound binomial", [&](t_result_type& s1, t_result_type& s2) { binomial_bench.benchmark_compound(10'000'000, s1, s2); });
}

std::int32_t main(std::int32_t /**argc*/, char** /**argv*/, char** /*envp*/)
{
    try
    {
        // ~~ Definitive tests ~~
        ropufu::test_aftermath::format_test formatter("./tests_temp.mat");
        //ropufu::test_aftermath::empirical_measure_test histogrammer;
        run_definitive_test("matstream v4", [&]() { return formatter.test_matstream_v4(10, 80, 80, 5); });
        run_definitive_test("empirical measure", [&]() { return formatter.test_matstream_v4(10, 80, 80, 5); });
        run_definitive_test("enum array 1", [&]() { return ropufu::test_aftermath::enum_array_test::basic_test<double>(); });
        run_definitive_test("enum array 2", [&]() { return ropufu::test_aftermath::enum_array_test::basic_test<std::size_t>(); });
        run_definitive_test("enum array 3", [&]() { return ropufu::test_aftermath::enum_array_test::basic_test_bool(); });
        run_definitive_test("enum array 4", [&]() { return ropufu::test_aftermath::enum_array_test::basic_test_void(); });

        run_definitive_test("fraction 1a", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_1<std::size_t>(); });
        run_definitive_test("fraction 1b", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_1<std::int_fast32_t>(); });
        run_definitive_test("fraction 1c", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_1<char>(); });
        run_definitive_test("fraction 2a", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_2<std::size_t>(); });
        run_definitive_test("fraction 2b", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_2<std::int_fast32_t>(); });
        run_definitive_test("fraction 2c", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_2<char>(); });
        run_definitive_test("fraction 3a", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_3<std::size_t>(); });
        run_definitive_test("fraction 3b", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_3<std::int_fast32_t>(); });
        run_definitive_test("fraction 3c", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_3<char>(); });
        run_definitive_test("fraction 4", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_4<std::size_t>(); });
        run_definitive_test("fraction 5a", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_5<std::size_t>(); });
        run_definitive_test("fraction 5b", [&]() { return ropufu::test_aftermath::algebra::fraction_tester::test_5<std::int_fast32_t>(); });
        

        std::cout << "Randomized tests with std::default_random_engine and double." << std::endl;
        // std::cout << "Randomized tests with std::default_random_engine: (i) float, then (ii) double." << std::endl;
        // not_main<float, std::default_random_engine>();
        not_main<double, std::default_random_engine>();

        std::cout << "Randomized tests with std::mt19937 and float." << std::endl;
        //std::cout << "Randomized tests with std::mt19937: (i) float, then (ii) double." << std::endl;
        not_main<float, std::mt19937>();
        //not_main<double, std::mt19937>();

        std::cout << "Randomized tests with std::mt19937_64 and float." << std::endl;
        //std::cout << "Randomized tests with std::mt19937_64: (i) float, then (ii) double." << std::endl;
        not_main<float, std::mt19937_64>();
        //not_main<double, std::mt19937_64>();
    } // try
    catch (const std::exception& ex)
    {
        std::cout << "~~ Oh no! ~~" << std::endl;
        std::cout << ex.what() << std::endl;
    } // catch (...)

    return 0;
} // main(...)
