
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../../ropufu/random/bernoulli_sampler.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/iid_process.hpp"

#include <array>      // std::array
#include <cmath>      // std::log
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <set>        // std::multiset
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_ALL_TYPES    \
    ropufu::aftermath::random::bernoulli_sampler<std::mt19937>,    \
    ropufu::aftermath::random::binomial_sampler<std::mt19937>,     \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937>,   \
    ropufu::aftermath::random::uniform_int_sampler<std::mt19937>   \

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing iid_process json", sampler_type, ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_ALL_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_process<sampler_type>;
    process_type proc{};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(proc, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

#ifndef ROPUFU_NO_JSON
TEST_CASE("testing iid_process json (Gaussian)")
{
    using sampler_type = ropufu::aftermath::random::normal_sampler_512<std::mt19937>;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::iid_process<sampler_type>;

    distributiont_type d{17, 29};
    process_type proc {d};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(proc, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE(...)
#endif

TEST_CASE_TEMPLATE("testing iid_process one-at-a-time", sampler_type, ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_ALL_TYPES)
{
    using process_type = ropufu::aftermath::sequential::iid_process<sampler_type>;
    static constexpr std::size_t count = 8;

    process_type proc {};
    for (std::size_t i = 0; i < count; ++i) proc.next();
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE("testing iid_process bulk (Gaussian)")
{
    using sampler_type = ropufu::aftermath::random::normal_sampler_512<std::mt19937>;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::iid_process<sampler_type>;
    using container_type = typename process_type::container_type;
    static constexpr std::size_t count = 17;

    distributiont_type d{17, 29};
    process_type proc {d};
    container_type values(count);
    proc.next(values);
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
