
#ifndef ROPUFU_AFTERMATH_BENCHMARK_HPP_INCLUDED
#define ROPUFU_AFTERMATH_BENCHMARK_HPP_INCLUDED

#include <chrono> // std::chrono::steady_clock, ...

namespace ropufu
{
    struct benchmark
    {
        using clock_type = std::chrono::steady_clock;
        using time_type = typename std::chrono::time_point<clock_type>;
        using duration_type = typename time_type::duration;

    private:
        time_type m_start = {};

    public:
        void tic() noexcept
        {
            this->m_start = std::chrono::steady_clock::now();
        } // tic(...)

        double toc() noexcept
        {
            time_type now = std::chrono::steady_clock::now();
            duration_type elapsed = now - this->m_start;
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            return static_cast<double>(milliseconds) / 1'000.00;
        } // toc(...)
    }; // struct benchmark
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_BENCHMARK_HPP_INCLUDED
