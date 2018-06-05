
#ifndef ROPUFU_AFTERMATH_TESTS_TEST_ALGEBRA_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_TEST_ALGEBRA_HPP_INCLUDED

#include "../ropufu/algebra.hpp"

#include <system_error> // std::error_code, std::errc
#include <type_traits>

namespace ropufu::test_aftermath::algebra
{
    struct fraction_tester
    {
        template <typename t_integer_type>
        using tested_t = ropufu::aftermath::algebra::fraction<t_integer_type>;

        template <typename t_integer_type>
        bool static test_1() noexcept
        {
            tested_t<t_integer_type> f{};
            return f == 0;
        } // test_1(...)

        template <typename t_integer_type>
        bool static test_2() noexcept
        {
            tested_t<t_integer_type> f{};
            f *= 15;
            return f == 0;
        } // test_2(...)

        template <typename t_integer_type>
        bool static test_3() noexcept
        {
            tested_t<t_integer_type> f{};
            f = f * f + f;
            return f == 0;
        } // test_3(...)

        template <typename t_integer_type>
        bool static test_4() noexcept
        {
            std::error_code ec {};
            tested_t<t_integer_type> f{1, 3, ec};
            if (ec) return false;
            tested_t<t_integer_type> g{2, 3, ec};
            if (ec) return false;
            return f + g == 1;
        } // test_4(...)

        template <typename t_integer_type>
        bool static test_5() noexcept
        {
            std::error_code ec {};
            tested_t<t_integer_type> f{4, 3, ec};
            if (ec) return false;
            tested_t<t_integer_type> g{2, 3, ec};
            if (ec) return false;
            tested_t<t_integer_type> h{5, 3, ec};
            if (ec) return false;
            tested_t<t_integer_type> t{2, 6, ec};
            if (ec) return false;
            if constexpr (std::is_signed_v<t_integer_type>) return f + (g - h) == t;
            else
            {
                return ((f + g) - h == t) && (g - h > 0);
            } // if constexpr
        } // test_5(...)
    }; // struct fraction_tester
} // namespace ropufu::test_aftermath::algebra

#endif // ROPUFU_AFTERMATH_TESTS_TEST_ALGEBRA_HPP_INCLUDED
