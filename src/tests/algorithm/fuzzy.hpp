
#ifndef ROPUFU_AFTERMATH_TESTS_DRAFT_FUZZY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_DRAFT_FUZZY_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algorithm/fuzzy.hpp"

#include <cstddef>  // std::size_t
#include <random>   // std::mt19937
#include <vector>   // std::vector

namespace ropufu::tests
{
    template <typename t_numeric_type>
    struct increasing_func_linear
    {
        using argument_type = t_numeric_type;
        using value_type = t_numeric_type;
        static constexpr bool is_increasing = true;
        static constexpr bool is_decreasing = false;

        static inline std::string name = "linear";
        value_type operator ()(argument_type x) const noexcept { return x; }
    }; // struct increasing_func_linear

    template <typename t_numeric_type>
    struct increasing_func_cubic
    {
        using argument_type = t_numeric_type;
        using value_type = t_numeric_type;
        static constexpr bool is_increasing = true;
        static constexpr bool is_decreasing = false;

        static inline std::string name = "cubic";
        value_type operator ()(argument_type x) const noexcept { return x * x * x; }
    }; // struct increasing_func_cubic

    template <typename t_numeric_type>
    struct decreasing_func_quadratic
    {
        using argument_type = t_numeric_type;
        using value_type = t_numeric_type;
        static constexpr bool is_increasing = false;
        static constexpr bool is_decreasing = true;

        static inline std::string name = "quadratic";
        value_type operator ()(argument_type x) const noexcept { return (x < 0) ? (x * x) : (-x * x); }
    }; // struct decreasing_func_quadratic
} // namespace ropufu::tests

#define ROPUFU_AFTERMATH_TESTS_ALGORITHM_FUZZY_FUNCTION_TYPES   \
    ropufu::tests::increasing_func_linear<float>,    \
    ropufu::tests::increasing_func_linear<double>,   \
    ropufu::tests::increasing_func_cubic<double>,    \
    ropufu::tests::decreasing_func_quadratic<double> \


TEST_CASE_TEMPLATE("testing fuzzy on increasing functions", function_t, ROPUFU_AFTERMATH_TESTS_ALGORITHM_FUZZY_FUNCTION_TYPES)
{
    using argument_type = typename function_t::argument_type;
    using value_type = argument_type;
    using fuzzy_type = ropufu::aftermath::algorithm::fuzzy<argument_type, value_type>;

    std::mt19937 engine {};
    ropufu::tests::seed(engine);
    value_type norm = static_cast<value_type>(std::mt19937::max()) + 1;
    value_type offset = static_cast<value_type>(std::mt19937::max() / 2);

    value_type error_magnitude = 0;
    SUBCASE("") { error_magnitude = value_type(0.5); }
    SUBCASE("") { error_magnitude = value_type(1.0); }
    SUBCASE("") { error_magnitude = value_type(4.0); }

    CAPTURE(function_t::name);
    CAPTURE(error_magnitude);

    std::error_code ec {};
    function_t f {};
    auto e = [&engine, offset, norm, error_magnitude]() { return error_magnitude * (engine() - offset) / norm; };
    auto g = [&f, &e] (argument_type x) { return f(x) + e(); };
    fuzzy_type not_fuzzy {f};
    fuzzy_type fuzzy {g};

    not_fuzzy.initialize_grid(-1, 0.5);
    not_fuzzy.options(2);

    argument_type lower_bound_not_fuzzy = 0;
    argument_type upper_bound_not_fuzzy = 0;
    if constexpr (function_t::is_increasing) not_fuzzy.find_zero_increasing(lower_bound_not_fuzzy, upper_bound_not_fuzzy, ec);
    if constexpr (function_t::is_decreasing) not_fuzzy.find_zero_decreasing(lower_bound_not_fuzzy, upper_bound_not_fuzzy, ec);
    REQUIRE(ec.value() == 0);
    REQUIRE(lower_bound_not_fuzzy <= 0);
    REQUIRE(upper_bound_not_fuzzy >= 0);

    fuzzy.initialize_grid(-1, 0.5);
    fuzzy.options(4);
    argument_type lower_bound_fuzzy = 0;
    argument_type upper_bound_fuzzy = 0;
    if constexpr (function_t::is_increasing) fuzzy.find_zero_increasing(lower_bound_fuzzy, upper_bound_fuzzy, ec);
    if constexpr (function_t::is_decreasing) fuzzy.find_zero_decreasing(lower_bound_fuzzy, upper_bound_fuzzy, ec);
    CHECK(ec.value() == 0);
    CHECK(lower_bound_fuzzy <= 0);
    CHECK(upper_bound_fuzzy >= 0);
} // TEST_CASE(...)

#endif // ROPUFU_AFTERMATH_TESTS_DRAFT_FUZZY_HPP_INCLUDED
