
#ifndef ROPUFU_AFTERMATH_TESTS_PROBABILITY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_PROBABILITY_HPP_INCLUDED

#include "../ropufu/probability.hpp"

#include <cmath>   // std::abs
#include <cstddef> // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>

namespace ropufu::aftermath::tests
{
    struct probability_tester
    {
        bool static test_normal_quantiles() noexcept
        {
            std::error_code ec {};
            probability::normal_distribution<double> norm {};

            if (std::abs(norm.quantile(0.50, ec) - 0.000000) > 0.0) return false;
            if (std::abs(norm.quantile(0.40, ec) + 0.253347) > 1e-5) return false;
            if (std::abs(norm.quantile(0.60, ec) - 0.253347) > 1e-5) return false;
            if (std::abs(norm.quantile(0.80, ec) - 0.841621) > 1e-5) return false;
            if (std::abs(norm.quantile(0.90, ec) - 1.28155) > 1e-5) return false;
            if (std::abs(norm.quantile(0.05, ec) + 1.64485) > 1e-5) return false;

            return true;
        } // test_normal_quantiles(...)
    }; // struct probability_tester
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_PROBABILITY_HPP_INCLUDED
