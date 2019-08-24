
#ifndef ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED

#include <doctest/doctest.h>

#include <cstddef>    // std::size_t
#include <ostream>    // std::ostream, std::endl
#include <string>     // std::string

#define BENCH_COMPARE_TIMING(left_milliseconds, right_milliseconds)                           \
    ropufu::aftermath::tests::benchmark_compare_timing(left_milliseconds, right_milliseconds) \


namespace ropufu::aftermath::tests
{
    static bool g_do_benchmarks = false;
    static std::size_t g_count_bench_successes = 0;
    static std::size_t g_count_bench_failures = 0;

    static void benchmark_compare_timing(double left_milliseconds, double right_milliseconds) noexcept
    {
        if (left_milliseconds < right_milliseconds) ++g_count_bench_successes;
        else ++g_count_bench_failures;
    } // benchmark_compare_timing(...)

    struct benchmark_reporter : public doctest::IReporter
    {
        // caching pointers/references to objects of these types - safe to do
        std::ostream& m_cout;
        const doctest::ContextOptions& m_options;

        // constructor has to accept the ContextOptions by ref as a single argument
        benchmark_reporter(const doctest::ContextOptions& in)
            : m_cout(*in.cout), m_options(in)
        {
            g_do_benchmarks = true;
        } // benchmark_reporter(...)

        void report_query(const doctest::QueryData& /*in*/) override { }

        void test_run_start() override { }

        void test_run_end(const doctest::TestRunStats& /*in*/) override
        {
            this->m_cout << doctest::Color::Yellow <<
                "===============================================================================" <<
                doctest::Color::None << std::endl;
            this->m_cout <<
                doctest::Color::Cyan << "[benchmark]" << doctest::Color::None <<
                " passed: " << g_count_bench_successes << "," <<
                " failed: " << g_count_bench_failures << "." << std::endl;
        } // test_run_end(...)

        void test_case_start(const doctest::TestCaseData& /*in*/) override { }

        void test_case_end(const doctest::CurrentTestCaseStats& /*in*/) override { }

        void test_case_exception(const doctest::TestCaseException& /*in*/) override { }

        void subcase_start(const doctest::SubcaseSignature& /*in*/) override { }

        void subcase_end() override { }

        void log_assert(const doctest::AssertData& /*in*/) override { }

        void log_message(const doctest::MessageData& /*in*/) override { }

        void test_case_skipped(const doctest::TestCaseData& /*in*/) override { }
    }; // struct benchmark_reporter
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED
