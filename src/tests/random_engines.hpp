
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_ENGINES_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_ENGINES_HPP_INCLUDED

#include "../ropufu/math_constants.hpp"

#include <cstddef>  // std::size_t
#include <iostream> // std::cout, std::endl
#include <random>   // ...
#include <set>      // std::set

namespace ropufu::aftermath::tests
{
    template <typename t_engine_type>
    std::size_t diameter_of() { return static_cast<std::size_t>(t_engine_type::max() - t_engine_type::min()); }

    static void print_random_engine_diameters() noexcept
    {
        std::cout << "===============================================================================" << std::endl;
        std::cout << "Common random engines:" << std::endl;
        std::cout << "ranlux24_base : " << std::ranlux24_base::min() << " -- " << std::ranlux24_base::max() << std::endl;
        std::cout << "     ranlux24 : " << std::ranlux24::min() << " -- " << std::ranlux24::max() << std::endl;
        std::cout << "  minstd_rand : " << std::minstd_rand::min() << " -- " << std::minstd_rand::max() << std::endl;
        std::cout << " minstd_rand0 : " << std::minstd_rand0::min() << " -- " << std::minstd_rand0::max() << std::endl;
        std::cout << "      knuth_b : " << std::knuth_b::min() << " -- " << std::knuth_b::max() << std::endl;
        std::cout << "      mt19937 : " << std::mt19937::min() << " -- " << std::mt19937::max() << std::endl;
        std::cout << "ranlux48_base : " << std::ranlux48_base::min() << " -- " << std::ranlux48_base::max() << std::endl;
        std::cout << "     ranlux48 : " << std::ranlux48::min() << " -- " << std::ranlux48::max() << std::endl;
        std::cout << "   mt19937_64 : " << std::mt19937_64::min() << " -- " << std::mt19937_64::max() << std::endl;

        std::set<std::size_t> diameters {};

        diameters.insert(diameter_of<std::mt19937>());
        diameters.insert(diameter_of<std::mt19937_64>());
        diameters.insert(diameter_of<std::minstd_rand>());
        diameters.insert(diameter_of<std::minstd_rand0>());
        diameters.insert(diameter_of<std::ranlux24_base>());
        diameters.insert(diameter_of<std::ranlux48_base>());
        diameters.insert(diameter_of<std::ranlux24>());
        diameters.insert(diameter_of<std::ranlux48>());
        diameters.insert(diameter_of<std::knuth_b>());

        std::cout << std::endl << "Diameters:" << std::endl;
        for (const std::size_t& x : diameters)
        {
            std::size_t count_bits = ropufu::aftermath::mersenne_power(x);
            std::cout << "    " << x;
            if (count_bits == 0) std::cout << " (non-Mersenne)";
            else std::cout << " (full " << count_bits << " bits)";
            std::cout << std::endl;
        } // for (...)
    } // print_random_engine_diameters(...)
} // namespace ropufu::aftermath::tests

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_ENGINES_HPP_INCLUDED