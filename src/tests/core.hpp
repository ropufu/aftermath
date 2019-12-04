
#ifndef ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include <array>   // std::array
#include <chrono>  // std::chrono::steady_clock, std::chrono::system_clock
#include <cstddef> // std::size_t
#include <list>    // std::list
#include <random>  // std::seed_seq
#include <sstream> // std::ostringstream
#include <string>  // std::string
#include <type_traits> // std::is_same_v
#include <vector>  // std::vector

namespace ropufu::aftermath::tests
{
    static int g_aux_test_counter = 0;

    template <typename t_engine_type>
    constexpr std::size_t engine_slowdown_factor([[maybe_unused]] std::size_t correction = 1) noexcept
    {
        if constexpr (std::is_same_v<t_engine_type, std::ranlux24>) return 1 + (20 / correction);
        else if constexpr (std::is_same_v<t_engine_type, std::ranlux48>) return 1 + (30 / correction);
        else return 1;
    } // engine_slowdown_factor(...)
    
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct engine_distribution_tuple
    {
        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;

        static constexpr std::string engine_name() noexcept
        {
            if constexpr (std::is_same_v<engine_type, std::ranlux24>) return "std::ranlux24";
            else if constexpr (std::is_same_v<engine_type, std::minstd_rand>) return "std::minstd_rand";
            else if constexpr (std::is_same_v<engine_type, std::mt19937>) return "std::mt19937";
            else if constexpr (std::is_same_v<engine_type, std::ranlux48>) return "std::ranlux48";
            else if constexpr (std::is_same_v<engine_type, std::mt19937_64>) return "std::mt19937_64";
            else return "unknown engine";
        } // engine_name(...)
    }; // struct engine_distribution_tuple

    template <typename t_engine_type>
    void seed(t_engine_type& engine) noexcept
    {
        int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
        std::seed_seq sequence{ 1, 1, 2, 3, 5, 8, 1729, time_seed, ++ropufu::aftermath::tests::g_aux_test_counter };
        engine.seed(sequence);
    } // seed(...)

    /** Performs action \p test and returns the number of seconds it took. */
    template <typename t_test_type>
    double benchmark(t_test_type&& test) noexcept
    {
        auto tic = std::chrono::steady_clock::now();
        test();
        auto toc = std::chrono::steady_clock::now();

        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count()) / 1'000.00;
    } // cusum_run_length(...)
    
    /** Samples from a distribution \p sample_size times and returns the number of seconds it took. */
    template <typename t_engine_type, typename t_sampler_type>
    double sample_timing(std::size_t sample_size, t_engine_type& engine, t_sampler_type& sampler) noexcept
    {
        using value_type = typename t_sampler_type::result_type;
        ropufu::aftermath::tests::seed(engine);

        return ropufu::aftermath::tests::benchmark(
            [&engine, &sampler, sample_size] () {
                if constexpr (std::is_same_v<value_type, bool>)
                {
                    std::size_t count = 0;
                    for (std::size_t k = 0; k < sample_size; ++k) if (sampler(engine)) ++count;
                } // if constexpr (...)
                else
                {
                    value_type sum = 0;
                    for (std::size_t k = 0; k < sample_size; ++k) sum += (sampler(engine));
                } // else (...)
            });
    } // timing_vs_builtin(...)
    
    template <typename t_left_type, typename t_right_type>
    struct type_pair
    {
        using left_type = t_left_type;
        using right_type = t_right_type;
    }; // struct type_pair
    
    template <typename t_left_type, typename t_middle_type, typename t_right_type>
    struct type_triplet
    {
        using left_type = t_left_type;
        using middle_type = t_middle_type;
        using right_type = t_right_type;
    }; // struct type_triplet

    template <typename t_type>
    bool does_json_round_trip(const t_type& x) noexcept
    {
        try
        {
            nlohmann::json j = x;
            t_type y = j;
            return x == y;
        } // try
        catch (...) { return false; }
    } // does_json_round_trip(...)

    template <typename t_type>
    bool are_ostream_equal(const t_type& x, const t_type& y) noexcept
    {
        try
        {
            std::ostringstream ssx { };
            std::ostringstream ssy { };

            ssx << x;
            ssy << y;
            return ssx.str() == ssy.str();
        } // try
        catch (...) { return false; }
    } // are_ostream_equal(...)
    
    template <typename t_container_type>
    static bool try_initialize_container(t_container_type& x, std::size_t size) noexcept
    {
        return false;
    } // try_initialize_container(...)

    template <typename t_numeric_type, std::size_t t_size>
    static bool try_initialize_container(std::array<t_numeric_type, t_size>& x, std::size_t /*size*/) noexcept
    {
        t_numeric_type seed {};
        for (std::size_t i = 0; i < x.size(); ++i) x[i] = ++seed;
        return true;
    } // try_initialize_container(...)

    template <typename t_numeric_type>
    static bool try_initialize_container(std::list<t_numeric_type>& x, std::size_t size) noexcept
    {
        t_numeric_type seed = static_cast<t_numeric_type>(size) + 1;
        for (std::size_t i = 0; i < size; ++i) x.push_back(--seed);
        return true;
    } // try_initialize_container(...)

    template <typename t_numeric_type>
    static bool try_initialize_container(std::vector<t_numeric_type>& x, std::size_t size) noexcept
    {
        t_numeric_type seed = static_cast<t_numeric_type>(size / 2);
        for (std::size_t i = 0; i < size; ++i) x.push_back(++seed);
        return true;
    } // try_initialize_container(...)
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
