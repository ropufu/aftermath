
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../../ropufu/random/bernoulli_sampler.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/iid_persistent_process.hpp"

#include <array>      // std::array
#include <cmath>      // std::log
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <set>        // std::multiset
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_ALL_TYPES    \
    ropufu::aftermath::random::bernoulli_sampler<std::mt19937>,    \
    ropufu::aftermath::random::binomial_sampler<std::mt19937>,     \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937>,   \
    ropufu::aftermath::random::uniform_int_sampler<std::mt19937>   \

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing iid_persistent_process json", sampler_type, ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_ALL_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    process_type proc{};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(proc, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

#ifndef ROPUFU_NO_JSON
TEST_CASE("testing iid_persistent_process json (Gaussian)")
{
    using sampler_type = ropufu::aftermath::random::normal_sampler_512<std::mt19937>;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;

    distributiont_type d1{17, 29};
    distributiont_type d2{1, 1};
    process_type proc {d1, d2, 2};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(proc, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE(...)
#endif

TEST_CASE_TEMPLATE("testing iid_persistent_process one-at-a-time", sampler_type, ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_ALL_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    static constexpr std::size_t count = 8;

    process_type proc {};
    for (std::size_t i = 0; i < count; ++i) proc.next();
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE("testing iid_persistent_process changed (Uniform)")
{
    using sampler_type = ropufu::aftermath::random::uniform_int_sampler<std::mt19937>;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    using value_type = typename process_type::value_type;

    // No-change: {17, ..., 29}.
    static constexpr value_type a = 17;
    static constexpr value_type b = 29;
    // Under-change: {1}.
    static constexpr value_type c = 1;
    static constexpr value_type d = 1;

    static constexpr std::size_t first_under_change_index = 2;
    static constexpr std::size_t count = 8;

    distributiont_type d1{a, b};
    distributiont_type d2{c, d};
    process_type proc {d1, d2, first_under_change_index};
    for (std::size_t i = 0; i < count; ++i)
    {
        auto x = proc.next();
        if (i >= first_under_change_index)
        {
            CHECK_GE(x, c);
            CHECK_LE(x, d);
        } // if (...)
        else
        {
            CHECK_GE(x, a);
            CHECK_LE(x, b);
        } // else
    } // for (...)
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE(...)

TEST_CASE("testing iid_persistent_process bulk (Gaussian)")
{
    using sampler_type = ropufu::aftermath::random::normal_sampler_512<std::mt19937>;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::iid_persistent_process<sampler_type, sampler_type>;
    using container_type = typename process_type::container_type;
    static constexpr std::size_t count = 17;

    distributiont_type d1{17, 29};
    distributiont_type d2{1, 1};
    process_type proc {d1, d2, 2};
    container_type values{count};
    proc.next(values);
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED
