
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/sequential/auto_regressive_process.hpp"

#include <array>      // std::array
#include <cmath>      // std::log
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <set>        // std::multiset
#include <stdexcept>  // std::logic_error

#ifdef ROPUFU_TMP_TEST_TYPES
#undef ROPUFU_TMP_TEST_TYPES
#endif
#define ROPUFU_TMP_TEST_TYPES                                      \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937>    \

#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing auto_regressive_process json", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::auto_regressive_process<sampler_type>;
    using container_type = typename process_type::container_type;

    distributiont_type d{};
    if constexpr (distributiont_type::parameter_dim == 1) d = distributiont_type(17);
    if constexpr (distributiont_type::parameter_dim == 2) d = distributiont_type(17, 29);
    container_type phi {static_cast<value_type>(0.1), static_cast<value_type>(-0.3), static_cast<value_type>(0.2)};

    process_type a{};
    process_type b{d, phi};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(b, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing auto_regressive_process one-at-a-time", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::auto_regressive_process<sampler_type>;
    using container_type = typename process_type::container_type;

    distributiont_type d{};
    if constexpr (distributiont_type::parameter_dim == 1) d = distributiont_type(17);
    if constexpr (distributiont_type::parameter_dim == 2) d = distributiont_type(17, 29);
    container_type phi {static_cast<value_type>(0.1), static_cast<value_type>(-0.3), static_cast<value_type>(0.2)};

    static constexpr std::size_t count = 8;

    process_type proc {d, phi};
    for (std::size_t i = 0; i < count; ++i) proc.next();
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing auto_regressive_process bulk", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using distributiont_type = typename sampler_type::distribution_type;
    using process_type = ropufu::aftermath::sequential::auto_regressive_process<sampler_type>;
    using container_type = typename process_type::container_type;

    distributiont_type d{};
    if constexpr (distributiont_type::parameter_dim == 1) d = distributiont_type(17);
    if constexpr (distributiont_type::parameter_dim == 2) d = distributiont_type(17, 29);
    container_type phi {static_cast<value_type>(0.1), static_cast<value_type>(-0.3), static_cast<value_type>(0.2)};

    static constexpr std::size_t count = 17;

    process_type proc {d, phi};
    container_type values(count);
    proc.next(values);
    CHECK_EQ(proc.count(), count);

    proc.clear();
    CHECK_EQ(proc.count(), 0);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED
