
#ifndef ROPUFU_AFTERMATH_TESTS_PATHFINDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PATHFINDER_HPP_INCLUDED

#include "../ropufu/algebra.hpp"
#include "../ropufu/algorithm.hpp"

#include <array>    // std::array
#include <cstddef>  // std::size_t
#include <cstdint>  // std::int32_t
#include <string>   // std::string, std::to_string
#include <system_error> // std::error_code, std::errc
#include <vector>   // std::vector

namespace ropufu::aftermath::tests
{
    using matrix_type = algebra::matrix<bool>;
    using index_type = algebra::matrix_index<std::size_t>;
    using projector_type = algorithm::matrix_projector_t<matrix_type>;
    using tested_type = algorithm::pathfinder<projector_type>;

    struct pathfinder_tester
    {
        static bool diagonal_test() noexcept
        {
            return true;
        } // diagonal_test(...)

        static bool horizontal_test() noexcept
        {
            std::error_code ec {};

            //    0  1  2  3
            // 0  o--o--o--o
            // 1  o--o--#--o
            // 2  x--#--x--o
            // 3  o--#--o--o
            // 4  o--o--o--o
            matrix_type m1 {5, 4};
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

            projector_type projector {m1, true};
            tested_type pathfinder {projector, source, ec};
            if (ec.value() != 0) return false;

            std::vector<index_type> path {};
            pathfinder.trace(target, path, ec);
            if (ec.value() != 0) return false;

            if (path.size() != 7) return false;
            if (path != shortest_path) return false;
            
            return true;
        } // horizontal_test(...)
    }; // struct pathfinder_tester
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_PATHFINDER_HPP_INCLUDED
