
#ifndef ROPUFU_AFTERMATH_TESTS_TEST_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_TEST_MATRIX_HPP_INCLUDED

#include "../ropufu/algebra.hpp"

#include <cstddef> // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>

namespace ropufu::test_aftermath::algebra
{
    struct matrix_tester
    {
        template <typename t_scalar_type>
        using tested_t = ropufu::aftermath::algebra::matrix<t_scalar_type>;

        template <typename t_left_scalar_type, typename t_right_scalar_type>
        static bool is_close(const tested_t<t_left_scalar_type>& left, const tested_t<t_right_scalar_type>& right, double tolerance = 0.01)
        {
            if (left.width() != right.width()) return false;
            if (left.height() != right.height()) return false;

            for (std::size_t i = 0; i < left.height(); ++i)
            {
                for (std::size_t j = 0; j < left.width(); ++j)
                {
                    double diff = static_cast<double>(left(i, j)) - static_cast<double>(right(i, j));
                    if (diff < 0) diff = -diff;
                    if (diff > tolerance) return false;
                } // for (...)
            } // for (...)

            return true;
        } // is_close(...)

        template <typename t_from_scalar_type, typename t_to_scalar_type>
        bool static test_matrix_cast(std::size_t height, std::size_t width) noexcept
        {
            tested_t<t_from_scalar_type> zero { height, width };
            tested_t<t_from_scalar_type> one { height, width };
            tested_t<t_from_scalar_type> b { height, width };
            tested_t<t_from_scalar_type> c { height, width };

            one.fill(1);
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) b(i, j) = static_cast<t_from_scalar_type>(i + (j % 2));
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) c(i, j) = static_cast<t_from_scalar_type>(b(i, j) + 1);
            
            tested_t<t_to_scalar_type> zero_cast { zero };
            tested_t<t_to_scalar_type> one_cast { one };
            tested_t<t_to_scalar_type> b_cast { b };
            tested_t<t_to_scalar_type> c_cast { c };

            if (!is_close(zero, zero_cast)) return false;
            if (!is_close(one, one_cast)) return false;
            if (!is_close(b, b_cast)) return false;
            if (!is_close(c, c_cast)) return false;
            return true;
        } // test_matrix_cast(...)

        template <typename t_scalar_type>
        bool static test_matrix_ops(std::size_t height, std::size_t width) noexcept
        {
            tested_t<t_scalar_type> zero { height, width };
            tested_t<t_scalar_type> one { height, width };
            tested_t<t_scalar_type> b { height, width };
            tested_t<t_scalar_type> c { height, width };

            one.fill(1);
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) b(i, j) = static_cast<t_scalar_type>(i + (j % 2));
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) c(i, j) = static_cast<t_scalar_type>(b(i, j) + 1);
            
            if (zero + one != one) return false;
            if (zero + b != b) return false;
            if (zero + c != c) return false;

            if (b + one != c) return false;

            tested_t<t_scalar_type> d = b;
            tested_t<t_scalar_type> e = c * one;
            tested_t<t_scalar_type> f = c / one;
            d *= zero;
            if (d != zero) return false;
            if (e != c) return false;
            if (f != c) return false;
            if (e != f) return false;

            return true;
        } // test_matrix_ops(...)

        template <typename t_scalar_type>
        bool static test_scalar_ops(std::size_t height, std::size_t width) noexcept
        {
            tested_t<t_scalar_type> zero { height, width };
            tested_t<t_scalar_type> one { height, width };
            tested_t<t_scalar_type> b { height, width };
            tested_t<t_scalar_type> c { height, width };

            t_scalar_type sc_zero = 0;
            t_scalar_type sc_one = 1;
            t_scalar_type sc_two = 2;

            one.fill(1);
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) b(i, j) = static_cast<t_scalar_type>(i + (j % 2));
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) c(i, j) = static_cast<t_scalar_type>(b(i, j) + 1);
            
            if (zero + sc_one != one) return false;
            if (one + sc_zero != one) return false;

            tested_t<t_scalar_type> bb = b;
            tested_t<t_scalar_type> cc = c;
            bb *= sc_two;
            cc += sc_two;

            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) if (bb(i, j) != b(i, j) * sc_two) return false;
            for (std::size_t i = 0; i < height; ++i) for (std::size_t j = 0; j < width; ++j) if (cc(i, j) != c(i, j) + sc_two) return false;

            if (b + sc_one != c) return false;

            tested_t<t_scalar_type> d = b;
            tested_t<t_scalar_type> e = c * sc_one;
            tested_t<t_scalar_type> f = c / sc_one;
            d *= sc_zero;
            if (d != zero) return false;
            if (e != c) return false;
            if (f != c) return false;

            return true;
        } // test_matrix_ops(...)
    }; // struct matrix_tester
} // namespace ropufu::test_aftermath::algebra

#endif // ROPUFU_AFTERMATH_TESTS_TEST_MATRIX_HPP_INCLUDED
