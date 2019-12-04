
#ifndef ROPUFU_AFTERMATH_RATIONALIZE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RATIONALIZE_HPP_INCLUDED

#include "number_traits.hpp"

#include <limits>      // std::numeric_limits
#include <type_traits> // std::is_floating_point_v
#include <stdexcept>   // std::logic_error

namespace ropufu::aftermath
{
    template <typename t_float_type, typename t_int_type, t_int_type t_diameter = std::numeric_limits<t_int_type>::max()>
    struct rationalize
    {
        using type = rationalize<t_float_type, t_int_type, t_diameter>;
        using float_type = t_float_type;
        using int_type = t_int_type;

        static constexpr int_type diameter = t_diameter;
        static constexpr float_type eps = std::numeric_limits<float_type>::epsilon();
        static constexpr int_type default_step = 1 + static_cast<int_type>(eps * diameter);

        //static constexpr bool does_roundtrip = (diameter == static_cast<int_type>(static_cast<float_type>(diameter))); // May issue overflow warning.

    private:
        static constexpr void traits_check() noexcept
        {
            static_assert(std::numeric_limits<int_type>::is_integer, "Int type has to be an integer type.");
            static_assert(std::is_floating_point_v<float_type>, "Float type has to be a floating point type.");
        } // traits_check(...)

        static constexpr int_type away_down(int_type current, float_type source, int_type step) noexcept
        {
            if (step == 0) step = 1;
            int_type x = current - step;
            while (static_cast<float_type>(x) == source)
            {
                if (x < step) return 0;
                x -= step;
            } // while(...)
            return x;
        } // away_down(...)

        static constexpr int_type toward_up(int_type current, float_type target, int_type step) noexcept
        {
            if (step == 0) step = 1;
            int_type x = current + step;
            while (static_cast<float_type>(x) != target)
            {
                if (x > type::diameter - step) return type::diameter;
                x += step;
            } // while(...)
            return x;
        } // toward_up(...)

    public:
        /** Finds the largest value below \p current that is represented differently as \tparam t_float_type. */
        static constexpr int_type below(int_type current) noexcept
        {
            type::traits_check();

            float_type mark = static_cast<float_type>(current);
            int_type step = (current < default_step) ? 1 : default_step;
            int_type x = current;
            while (step != 0)
            {
                x = away_down(x, mark, step); // x is now stricly below the target.
                x = toward_up(x, mark, step / 2); // x is now at the target.
                step /= 4;
            } // while (...)
            return away_down(x, mark, 1);
        } // below(...)

        /** @brief Converts \p p to a \tparam t_int_type representation in {0, 1, ..., diameter}.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         */
        static int_type probability(float_type p)
        {
            type::traits_check();
            if (!aftermath::is_finite(p) || p < 0 || p > 1)
                throw std::logic_error("Probability must be a finite number between 0 and 1.");

            constexpr float_type one = 1;
            constexpr float_type two = 2;
            constexpr float_type half = one / two;
            constexpr float_type denominator = static_cast<float_type>(type::diameter);
            if (2 * p > 1)
            {
                float_type numerator_q = denominator * (1 - p);
                int_type gap = static_cast<int_type>(numerator_q + half);
                return (type::diameter - gap);
            } // if (...)
            else
            {
                float_type numerator_p = denominator * p;
                int_type gap = static_cast<int_type>(numerator_p + half);
                return (gap);
            } // else (...)
        } // probability(...)
    }; // struct rationalize
} // ropufu::aftermath

#endif // ROPUFU_AFTERMATH_RATIONALIZE_HPP_INCLUDED
