
#ifndef ROPUFU_AFTERMATH_TESTS_JSON_SCHEMA_TO_HPP_GENERATED_CODE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_JSON_SCHEMA_TO_HPP_GENERATED_CODE_HPP_INCLUDED
#ifndef ROPUFU_NO_JSON

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include "../core.hpp"
#include "test_a.hpp"
#include "test_b.hpp"
#include "test_c.hpp"
#include "test_d.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int16_t, std::int32_t, std::int64_t
#include <functional> // std::hash
#include <string>     // std::string

#define ROPUFU_AFTERMATH_TESTS_JSON_SCHEMA_TO_HPP_TRIPLET_TYPES \
    ropufu::tests::type_triplet<std::int32_t, void, float>,     \
    ropufu::tests::type_triplet<std::int32_t, void, double>,    \
    ropufu::tests::type_triplet<std::uint16_t, void, float>,    \
    ropufu::tests::type_triplet<std::uint16_t, void, double>,   \
    ropufu::tests::type_triplet<std::size_t, void, double>      \

TEST_CASE("generated json roundtrip schema A")
{
    using test_a_type = ropufu::tests::json_schema_to_hpp::test_a;

    test_a_type x {1729};
    test_a_type y {};
    y.set_label("Meaow");

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(x, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(y, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE(...)

TEST_CASE_TEMPLATE("generated json roundtrip schema B", integer_type, std::int16_t, std::int32_t, std::uint32_t, std::size_t)
{
    using test_a_type = ropufu::tests::json_schema_to_hpp::test_a;
    using test_b_type = ropufu::tests::json_schema_to_hpp::test_b<integer_type>;

    test_b_type x {{"Woof"}};
    test_b_type y {};
    y.set_tag(test_a_type{88});

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(x, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(y, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing generated json roundtrip schema C", triplet_type, ROPUFU_AFTERMATH_TESTS_JSON_SCHEMA_TO_HPP_TRIPLET_TYPES)
{
    using integer_type = typename triplet_type::left_type;
    using probability_type = typename triplet_type::right_type;

    using test_a_type = ropufu::tests::json_schema_to_hpp::test_a;
    using test_c_type = ropufu::tests::json_schema_to_hpp::test_c<integer_type, probability_type>;
    
    test_c_type x {};
    x.set_chances({static_cast<probability_type>(0.1), static_cast<probability_type>(0.75)});
    x.set_age(21);
    test_c_type y {};
    y.set_tag(test_a_type{88});

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(x, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(y, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE

TEST_CASE("generated json roundtrip schema D")
{
    using test_d_type = ropufu::tests::json_schema_to_hpp::test_d;

    test_d_type x = test_d_type::one_one_one;
    test_d_type y = "two?";
    test_d_type z = "three";

    CHECK_EQ(x.hello_world(), true);
    CHECK_EQ(y.hello_world(), true);
    CHECK_EQ(z.hello_world(), true);

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(x, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(y, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(z, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE(...)

#endif // ROPUFU_NO_JSON
#endif // ROPUFU_AFTERMATH_TESTS_JSON_SCHEMA_TO_HPP_GENERATED_CODE_HPP_INCLUDED
