
#include "../aftermath/probability.hpp"
#include "../aftermath/random.hpp"

#include "definitive_tests.hpp"
#include "random_examples.hpp"
#include "random_test.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream> // For std::cout etc.
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

template <typename t_test_type>
void run_benchmark(const std::string& name, t_test_type&& test)
{
    double time_tested = 0;
    double time_builtin = 0;
    test(time_tested, time_builtin);
    std::cout << "Elapsed time " << time_tested << " vs. " << time_builtin << " seconds (tested vs. built-in) in " << name << "." << std::endl;
}

std::int32_t main(std::int32_t argc, char* argv[], char* envp[])
{
    using dist_normal = ropufu::aftermath::probability::dist_normal;
    using dist_lognormal = ropufu::aftermath::probability::dist_lognormal;

    ropufu::test_aftermath::format_test formatter("./tests.temp.mat");
    //ropufu::test_aftermath::empirical_measure_test histogrammer;
    ropufu::test_aftermath::test_random<dist_lognormal> lognormal_tester(0.0, 1.0);
    ropufu::test_aftermath::test_random<dist_normal> normal_tester(0.0, 1.0);
    ropufu::test_aftermath::binomial_benchmark<> binomial_bench(10, 11, 0.8);

    // ~~ Definitive tests ~~
    run_definitive_test("matstream v4", [&]() { return formatter.test_matstream_v4(10, 80, 80, 5); });
    run_definitive_test("empirical measure", [&]() { return formatter.test_matstream_v4(10, 80, 80, 5); });

    // ~~ Randomized tests ~~
    run_error_test("lognormal tail probability", [&]() { return lognormal_tester.error_in_tail(10'000'000, 8.5); });
    run_error_test("gaussian tail probability", [&]() { return normal_tester.error_in_tail(10'000'000, 3.0); });
    run_error_test("gaussian CUSUM", [&]() { return normal_tester.error_in_cusum(10'000, std::log(159.36)); });

    // ~~ Benchmarks ~~
    run_benchmark("lognormal tail probability", [&](double& s1, double& s2) { lognormal_tester.benchmark_tail(100'000'000, 8.5, s1, s2); });
    run_benchmark("gaussian tail probability", [&](double& s1, double& s2) { normal_tester.benchmark_tail(100'000'000, 3.0, s1, s2); });
    run_benchmark("gaussian CUSUM", [&](double& s1, double& s2) { normal_tester.benchmark_cusum(100'000, std::log(159.36), s1, s2); });
    run_benchmark("compound binomial", [&](double& s1, double& s2) { binomial_bench.benchmark_compound(10'000'000, s1, s2); });

    return 0;
}
