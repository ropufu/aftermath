
#ifndef ROPUFU_AFTERMATH_DISCREPANCY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_DISCREPANCY_HPP_INCLUDED

#include <concepts>    // std::floating_point
#include <ranges>      // std::ranges:range

namespace ropufu::aftermath
{
    namespace detail
    {
        template <typename t_value_type>
        struct discrepancy;

        template <std::floating_point t_value_type>
        struct discrepancy<t_value_type>
        {
            using result_type = t_value_type;
            using argument_type = t_value_type;

            result_type operator ()(const argument_type& x, const argument_type& y) const noexcept
            {
                argument_type diff = (x > y) ? (x - y) : (y - x);
                if (diff == 0) return 0;

                argument_type xx = (x > 0) ? (x) : (-x);
                argument_type yy = (y > 0) ? (y) : (-y);
                argument_type norm = (xx > yy) ? (xx) : (yy);

                diff /= norm;
                return diff;
            } // operator ()(...)
        }; // struct discrepancy<...>

        template <std::ranges::range t_range_type>
            requires std::floating_point<typename t_range_type::value_type>
        struct discrepancy<t_range_type>
        {
            using result_type = typename t_range_type::value_type;
            using argument_type = t_range_type;

            result_type operator ()(const argument_type& x, const argument_type& y) const noexcept
            {
                result_type total = 0;

                auto xi = std::ranges::cbegin(x);
                auto yi = std::ranges::cbegin(y);

                auto xx = std::ranges::cend(x);
                auto yy = std::ranges::cend(y);

                ropufu::aftermath::detail::discrepancy<result_type> scalar {};
                while (xi != xx && yi != yy)
                {
                    total += scalar(*xi, *yi);
                    ++xi;
                    ++yi;
                } // while (...)

                if (xi != xx || yi != yy) return 1729; // Size mismatch.
                return total;
            } // operator ()(...)
        }; // struct discrepancy<...>
    } // namespace detail

    template <typename t_value_type>
    inline auto discrepancy(const t_value_type& x, const t_value_type& y) noexcept
    {
        return detail::discrepancy<t_value_type>()(x, y);
    } // discrepancy(...)
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_DISCREPANCY_HPP_INCLUDED
