
#ifndef ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../ropufu/noexcept_json.hpp"
#include "../ropufu/metadata.hpp"

#include <array>   // std::array
#include <chrono>  // std::chrono::steady_clock, std::chrono::system_clock
#include <cstddef> // std::size_t
#include <list>    // std::list
#include <random>  // std::seed_seq
#include <sstream> // std::ostringstream
#include <string>  // std::string
#include <string_view> // std::string_view
#include <type_traits> // std::is_same_v
#include <vector>  // std::vector

namespace ropufu
{
    ROPUFU_MAKE_METADATA(std::ranlux24, "::std::ranlux24", false)
    ROPUFU_MAKE_METADATA(std::ranlux48, "::std::ranlux48", false)
    ROPUFU_MAKE_METADATA(std::minstd_rand, "::std::minstd_rand", false)
    ROPUFU_MAKE_METADATA(std::mt19937, "::std::mt19937", false)
    ROPUFU_MAKE_METADATA(std::mt19937_64, "::std::mt19937_64", false)
} // namespace ropufu

namespace ropufu::tests
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

        static constexpr std::string_view engine_name() noexcept { return ropufu::qualified_name<t_engine_type>(); }
    }; // struct engine_distribution_tuple

    template <typename t_engine_type>
    void seed(t_engine_type& engine) noexcept
    {
        int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
        std::seed_seq sequence{ 1, 1, 2, 3, 5, 8, 1729, time_seed, ++ropufu::tests::g_aux_test_counter };
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
    } // benchmark(...)
    
    /** Samples from a distribution \p sample_size times and returns the number of seconds it took. */
    template <typename t_engine_type, typename t_sampler_type>
    double sample_timing(std::size_t sample_size, t_engine_type& engine, t_sampler_type& sampler) noexcept
    {
        using value_type = typename t_sampler_type::result_type;
        ropufu::tests::seed(engine);

        return ropufu::tests::benchmark(
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
    } // sample_timing(...)
    
    template <typename t_left_type, typename t_middle_type, typename t_right_type>
    struct type_triplet
    {
        using left_type = t_left_type;
        using middle_type = t_middle_type;
        using right_type = t_right_type;
    }; // struct type_triplet

    template <typename t_type>
    bool does_json_round_trip(const t_type& x, std::string& a, std::string& b) noexcept
    {
        a = "Processing...";
        b = "Processing...";

        nlohmann::json j = x;
        a = j.dump();

        t_type y {};
        if (!noexcept_json::try_get(j, y)) return false;

        nlohmann::json k = y;
        b = k.dump();

        return x == y;
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

    template <typename t_left_matrix_type, typename t_right_matrix_type>
    long double matrix_distance(const t_left_matrix_type& left, const t_right_matrix_type& right) noexcept
    {
        long double result = std::numeric_limits<long double>::infinity();
        if (left.width() != right.width()) return result;
        if (left.height() != right.height()) return result;

        result = 0;
        for (std::size_t i = 0; i < left.height(); ++i)
        {
            for (std::size_t j = 0; j < left.width(); ++j)
            {
                long double diff = static_cast<long double>(left(i, j)) - static_cast<long double>(right(i, j));
                if (diff < 0) diff = -diff;
                if (diff > result) result = diff;
            } // for (...)
        } // for (...)

        return result;
    } // matrix_distance(...)

    template <typename t_matrix_type>
    t_matrix_type zeros_matrix(std::size_t height, std::size_t width) noexcept
    {
        return t_matrix_type(height, width);
    } // zeros_matrix(...)

    template <typename t_matrix_type>
    t_matrix_type ones_matrix(std::size_t height, std::size_t width) noexcept
    {
        return t_matrix_type(height, width, 1);
    } // ones_matrix(...)

    template <typename t_matrix_type>
    t_matrix_type non_negative_matrix_b(std::size_t height, std::size_t width) noexcept
    {
        using scalar_t = typename t_matrix_type::value_type;
        t_matrix_type result{ height, width };

        for (std::size_t i = 0; i < height; ++i)
            for (std::size_t j = 0; j < width; ++j)
                result(i, j) = static_cast<scalar_t>(i + (j % 2));

        return result;
    } // non_negative_matrix_b(...)
} // namespace ropufu::tests

#endif // ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
