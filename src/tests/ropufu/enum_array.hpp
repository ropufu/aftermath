
#ifndef ROPUFU_AFTERMATH_TESTS_ROPUFU_ENUM_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ROPUFU_ENUM_ARRAY_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <doctest/doctest.h>
#include <nlohmann/json.hpp>
#endif

#include "../../ropufu/enum_array.hpp"
#include "../core.hpp"
#include "../mystruct.hpp"

#include <array>    // std::array
#include <cstddef>  // std::size_t
#include <cstdint>  // std::int32_t
#include <string>   // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

#define ROPUFU_AFTERMATH_TESTS_ENUM_ARRAY_ALL_TYPES                      \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, double>,      \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, std::size_t>, \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, bool>,        \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, void>         \

#define ROPUFU_AFTERMATH_TESTS_ENUM_ARRAY_MAP_TYPES                      \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, double>,      \
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, std::size_t>  \


#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing enum_array json", tested_t, ROPUFU_AFTERMATH_TESTS_ENUM_ARRAY_MAP_TYPES)
{
    tested_t tested {};
    typename tested_t::underlying_type i = 0;
    for (auto x : tested) x.value() = ++i;

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(tested, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing enum_array flags", tested_t,
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, bool>)
{
    using enum_type = ropufu::tests::mystruct;
    using underlying_type = typename tested_t::underlying_type;

    tested_t tested { enum_type::almost_surely, enum_type::surely };
    underlying_type aggregate {};

    tested.set(enum_type::maybe);
    tested.set(enum_type::perhaps);
    tested.unset(enum_type::almost_surely);
    tested.unset(enum_type::maybe);

    for (enum_type x : tested) aggregate += static_cast<underlying_type>(x);
    underlying_type expected_aggregate = static_cast<underlying_type>(enum_type::perhaps) + static_cast<underlying_type>(enum_type::surely);

    CHECK_EQ(aggregate, expected_aggregate);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing enum_array void", tested_t,
    ropufu::aftermath::enum_array<ropufu::tests::mystruct, void>)
{
    using enum_type = ropufu::tests::mystruct;
    using underlying_type = typename tested_t::underlying_type;

    tested_t tested {};
    underlying_type aggregate {};

    for (enum_type x : tested) aggregate += static_cast<underlying_type>(x);
    underlying_type expected_aggregate = static_cast<underlying_type>(enum_type::maybe) +
        static_cast<underlying_type>(enum_type::perhaps) +
        static_cast<underlying_type>(enum_type::almost_surely) +
        static_cast<underlying_type>(enum_type::surely);
    
    CHECK_EQ(aggregate, expected_aggregate);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ROPUFU_ENUM_ARRAY_HPP_INCLUDED
