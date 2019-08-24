
#ifndef ROPUFU_AFTERMATH_TESTS_ALGORITHM_PATHFINDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_ALGORITHM_PATHFINDER_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../../ropufu/algebra/matrix_index.hpp"
#include "../../ropufu/algebra/matrix.hpp"
#include "../../ropufu/algorithm/pathfinder.hpp"
#include "../../ropufu/algorithm/projector.hpp"

#include <cstddef>  // std::size_t
#include <system_error> // std::error_code
#include <vector>   // std::vector

TEST_CASE("testing pathfinder tracing")
{
    using matrix_type = ropufu::aftermath::algebra::matrix<bool>;
    using index_type = ropufu::aftermath::algebra::matrix_index<std::size_t>;
    using projector_type = ropufu::aftermath::algorithm::matrix_projector_t<matrix_type>;
    using tested_type = ropufu::aftermath::algorithm::pathfinder<projector_type>;
    std::error_code ec {};

    //    0  1  2  3
    // 0  o--o--o--o
    // 1  o--o--#--o
    // 2  x--#--x--o
    // 3  o--#--o--o
    // 4  o--o--o--o
    projector_type projector {5, 4};
    projector.set_blocked_indicator(true);
    matrix_type& m1 = projector.surface();
    m1(1, 2) = true;
    m1(2, 1) = true;
    m1(3, 1) = true;
    index_type source {2, 0};
    index_type target {2, 2};
    std::vector<index_type> shortest_path {
        {2, 0},
        {3, 0},
        {4, 0},
        {4, 1},
        {4, 2},
        {3, 2},
        {2, 2}
    };

    tested_type pathfinder {projector, source};

    std::vector<index_type> path {};
    pathfinder.trace(target, path, ec);
    REQUIRE(ec.value() == 0);
    REQUIRE(path.size() == 7);
    CHECK(path == shortest_path);
} // TEST_CASE(...)

#endif // ROPUFU_AFTERMATH_TESTS_ALGORITHM_PATHFINDER_HPP_INCLUDED
