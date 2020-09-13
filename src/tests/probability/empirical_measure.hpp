
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../mystruct.hpp"
#include "../../ropufu/probability/empirical_measure.hpp"

#include <array>      // std::array
#include <cmath>      // std::abs, std::round
#include <cstddef>    // std::size_t
#include <cstdint>    // std::int16_t, std::int32_t, std::int64_t
#include <limits>     // std::numeric_limits
#include <stdexcept>  // std::logic_error
#include <string>     // std::string
#include <unordered_map> // std::unordered_map
#include <vector>     // std::vector

namespace ropufu::tests
{
    template <typename t_count_type>
    static void dictionary(std::vector<mystruct>& keys, std::vector<t_count_type>& counts)
    {
        keys = { mystruct::almost_surely, mystruct::perhaps, mystruct::maybe };
        counts = { t_count_type(15), t_count_type(5), t_count_type(0) };
    } // dictionary(...)

    template <typename t_count_type>
    static void dictionary(std::vector<std::string>& keys, std::vector<t_count_type>& counts)
    {
        keys = { "Hello", "world", "1729" };
        counts = { t_count_type(7), t_count_type(3), t_count_type(10) };
    } // dictionary(...)
} // namespace ropufu::tests

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_ORDERED_TYPES                \
    ropufu::aftermath::probability::empirical_measure<std::int16_t, std::int16_t, float>, \
    ropufu::aftermath::probability::empirical_measure<float, double, double>,             \
    ropufu::aftermath::probability::empirical_measure<std::size_t, float, double>,        \
    ropufu::aftermath::probability::empirical_measure<double, double, double>             \

#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_UNORDERED_TYPES              \
    ropufu::aftermath::probability::empirical_measure<                                    \
        ropufu::tests::mystruct, std::size_t>,                                 \
    ropufu::aftermath::probability::empirical_measure<std::string, std::int32_t>          \


TEST_CASE_TEMPLATE("testing empirical_measure ordered", tested_t, ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_ORDERED_TYPES)
{
    using key_type = typename tested_t::key_type;
    using count_type = typename tested_t::count_type;
    
    std::unordered_map<key_type, count_type> dictionary =
        {{key_type(1), count_type(10)}, {key_type(2), count_type(3)}, {key_type(0), count_type(7)}};
    std::vector<key_type> keys = {key_type(1), key_type(2), key_type(0)};
    std::vector<count_type> counts = {count_type(10), count_type(3), count_type(7)}; // Total of 20.
    std::vector<count_type> thousandths = {count_type(500), count_type(150), count_type(350)}; // Point probabilities expressed as thousandth points.

    tested_t a {};
    tested_t b {dictionary};
    tested_t c {keys, counts};

    REQUIRE(a.empty());
    REQUIRE(b.count() == c.count());
    REQUIRE(b.data() == c.data());

    a.observe(2, 3);
    REQUIRE(a.most_likely_value() == 2);
    REQUIRE(a.min() == 2);
    REQUIRE(a.max() == 2);

    a.observe(0, 7);
    REQUIRE(a.count() + 10 == b.count());
    REQUIRE(a.pmf(1) == 0);
    REQUIRE(a.most_likely_value() == 0);
    REQUIRE(a.min() == 0);
    REQUIRE(a.max() == 2);

    a.observe(1, 5);
    a.observe(1);
    a.observe(1, 4);
    REQUIRE(a.count() == b.count());
    REQUIRE(a.most_likely_value() == 1);
    REQUIRE(a.min() == 0);
    REQUIRE(a.max() == 2);

    for (std::size_t i = 0; i < keys.size(); ++i)
    {
        key_type x = keys[i];
        count_type t = thousandths[i];

        CHECK(static_cast<count_type>(std::round(a.pmf(x) * 1000)) == t);
        CHECK(static_cast<count_type>(std::round(b.pmf(x) * 1000)) == t);
        CHECK(static_cast<count_type>(std::round(c.pmf(x) * 1000)) == t);
    } // for (...)

    c.clear();
    REQUIRE(c.empty());

    c.merge(b);
    for (key_type x : keys) REQUIRE(c.pmf(x) == b.pmf(x));
    CHECK(c.mean() == doctest::Approx(0.8));
} // TEST_CASE_TEMPLATE(...)

TEST_CASE("testing empirical_measure unordered")
{
    using tested_type_a = ropufu::aftermath::probability::empirical_measure<std::string, std::int32_t>;
    using tested_type_b = ropufu::aftermath::probability::empirical_measure<ropufu::tests::mystruct, std::size_t>;
    
    tested_type_a a {};
    tested_type_b b {};

    a << "Hello, " << std::string("World!");
    REQUIRE(a.count() == 2);
    CHECK(a.most_likely_value() == "Hello, ");
    
    b << ropufu::tests::mystruct::maybe <<
        ropufu::tests::mystruct::almost_surely;
    b.observe(ropufu::tests::mystruct::maybe);
    REQUIRE(b.most_likely_count() == 2);
    REQUIRE(b.most_likely_value() == ropufu::tests::mystruct::maybe);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
