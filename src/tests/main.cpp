
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "benchmark_reporter.hpp"

// Tests:
#include "algebra/elementwise.hpp"
#include "algebra/fraction.hpp"
#include "algebra/matrix.hpp"
#include "algebra/range.hpp"
#include "algorithm/fuzzy.hpp"
#include "algorithm/pathfinder.hpp"
#include "format/mat4_stream_base.hpp"
#include "probability/empirical_measure.hpp"
#include "probability/exponential_distribution.hpp"
#include "probability/normal_distribution.hpp"
#include "probability/standard_exponential_distribution.hpp"
#include "probability/standard_normal_distribution.hpp"
#include "random/normal_sampler_512.hpp"
#include "ropufu/enum_array.hpp"

//#include "random_engines.hpp"

#include <exception> // std::exception
#include <iostream>  // std::cout, std::endl

REGISTER_REPORTER("bench", 1, ropufu::aftermath::tests::benchmark_reporter);
REGISTER_REPORTER("benchmark", 1, ropufu::aftermath::tests::benchmark_reporter);

int main(int argc, char** argv, char** /*envp*/)
{
    doctest::Context context {};
    try
    {
        context.applyCommandLine(argc, argv);
        int res = context.run();
        if (context.shouldExit()) return res;
    } // try
    catch (const std::exception& ex)
    {
        std::cout << "~~ Oh no! ~~" << std::endl;
        std::cout << ex.what() << std::endl;
    } // catch (...)

    // std::cout << std::endl;
    // ropufu::aftermath::tests::print_random_engine_diameters();
    return 0;
} // main(...)
