
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/window_limited_cusum.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int64_t
#include <random>  // std::mt19937_64
#include <string>  // std::string
#include <vector>  // std::vector

#ifdef ROPUFU_TMP_TEST_TYPES
#undef ROPUFU_TMP_TEST_TYPES
#endif
#define ROPUFU_TMP_TEST_TYPES                                                      \
    ropufu::aftermath::random::binomial_sampler<std::mt19937_64, std::int64_t>,    \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937_64, double>,        \
    ropufu::aftermath::random::uniform_int_sampler<std::mt19937_64, std::int64_t>  \


#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing window_limited_cusum json", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using window_limited_cusum_type = ropufu::aftermath::sequential::window_limited_cusum<value_type>;
    
    window_limited_cusum_type window_limited_cusum_o{};
    window_limited_cusum_type window_limited_cusum_a{5};
    window_limited_cusum_type window_limited_cusum_b{10};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(window_limited_cusum_o, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(window_limited_cusum_a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(window_limited_cusum_b, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing window_limited_cusum accumulation", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using window_limited_cusum_type = ropufu::aftermath::sequential::window_limited_cusum<value_type>;
    window_limited_cusum_type window_limited_cusum{5};

    std::vector<value_type> process = {2, 3, -7, 1, 2, 3, 4, 5, 5, -5};
    value_type r = 0;
    value_type s = 0;
    for (value_type x : process)
    {
        r = s;
        s = window_limited_cusum.observe(x);
    } // for (...)

    CHECK_EQ(r, 19);
    CHECK_EQ(s, 12);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
