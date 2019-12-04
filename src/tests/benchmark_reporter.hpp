
#ifndef ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED

#include <doctest/doctest.h>

#include <cmath>      // std::round
#include <cstddef>    // std::size_t
#include <mutex>      // std::mutex
#include <ostream>    // std::ostream, std::endl
#include <string>     // std::string

#define BENCH_COMPARE_TIMING(suffix, left_name, right_name, left_seconds, right_seconds)                           \
    ropufu::aftermath::tests::benchmark_compare_timing(suffix, left_name, right_name, left_seconds, right_seconds) \


namespace ropufu::aftermath::tests
{
    static constexpr int bench_percentage_threshold = 10;

    static bool g_do_benchmarks = false;
    static std::size_t g_count_bench_successes = 0;
    static std::size_t g_count_bench_ties = 0;
    static std::size_t g_count_bench_failures = 0;

    static void benchmark_compare_timing(const std::string& suffix,
        const std::string& left_name, const std::string& right_name,
        double left_seconds, double right_seconds) noexcept
    {
        double avg = (left_seconds + right_seconds) / 2;
        double percentage = 100 * (right_seconds - left_seconds) / avg;
        int code = static_cast<int>(std::round(percentage));

        if (code > tests::bench_percentage_threshold) ++g_count_bench_successes;
        else if (code < -tests::bench_percentage_threshold) ++g_count_bench_failures;
        else ++g_count_bench_ties;

        ADD_MESSAGE_AT(suffix.c_str(), code,
            left_name << ": " << left_seconds << " s vs. " <<
            right_name << ": " << right_seconds << " s.");
    } // benchmark_compare_timing(...)

    struct benchmark_reporter : public doctest::IReporter
    {
        // caching pointers/references to objects of these types - safe to do
        std::ostream& m_cout;
        const doctest::ContextOptions& m_options;
        const doctest::TestCaseData* m_test_ptr = nullptr;
        std::mutex m_mutex = {};

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
                " tied: " << g_count_bench_ties << "," <<
                " failed: " << g_count_bench_failures << "." << std::endl;
        } // test_run_end(...)

        void test_case_start(const doctest::TestCaseData& in) override { this->m_test_ptr = &in; }

        void test_case_end(const doctest::CurrentTestCaseStats& /*in*/) override { this->m_test_ptr = nullptr; }

        void test_case_exception(const doctest::TestCaseException& /*in*/) override { }

        void subcase_start(const doctest::SubcaseSignature& /*in*/) override { }

        void subcase_end() override { }

        void log_assert(const doctest::AssertData& /*in*/) override { }

        void log_message(const doctest::MessageData& in) override
        {
            std::lock_guard<std::mutex> lock(this->m_mutex);
            int percentage = in.m_line;
            std::string suffix = in.m_file;
            const std::string& test_name = this->m_test_ptr->m_name;

            this->m_cout << "[";
            if (percentage > tests::bench_percentage_threshold) this->m_cout << doctest::Color::BrightGreen;
            if (percentage < -tests::bench_percentage_threshold) this->m_cout << doctest::Color::BrightRed;
            this->m_cout << test_name << doctest::Color::None << " / " << doctest::Color::LightGrey <<
                suffix << doctest::Color::None << "] " <<
                in.m_string << std::endl;
        } // log_message(...)

        void test_case_skipped(const doctest::TestCaseData& /*in*/) override { }
    }; // struct benchmark_reporter
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_BENCHMARK_REPORTER_HPP_INCLUDED
