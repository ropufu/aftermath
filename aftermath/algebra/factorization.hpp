
#ifndef ROPUFU_AFTERMATH_ALGEBRA_FACTORIZATION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_FACTORIZATION_HPP_INCLUDED

#include <initializer_list>

namespace ropufu
{
    namespace aftermath
    {
        namespace algebra
        {
			/** @brief Factorization-related mathematics.
             *  @todo Rewrite to template on integer types only.
             */
            struct factorization
            {
                /** @brief Computes the greatest common divider (GCD) for two numbers. 
                 *  @todo Check for negative values.
                 */
                template <typename t_value_type>
                static t_value_type greatest_common_divisor(t_value_type a, t_value_type b) noexcept
                {
                    if (a == 0) return b;
                    if (b == 0) return a;

                    if (a > b) return factorization::greatest_common_divisor(a - b * (a / b), b);
                    if (b > a) return factorization::greatest_common_divisor(a, b - a * (b / a));
                    return a;
                }

                /** @brief Computes the greatest common divider (GCD) for a list of numbers.
                 *  @todo Check for negative values.
                 */
                template <typename t_value_type>
                static t_value_type greatest_common_divisor(std::initializer_list<t_value_type> a) noexcept
                {
                    auto it = a.begin();
                    if (it == a.end()) return 1;

                    t_value_type result = *it;
                    ++it;
                    for (; it != a.end(); ++it) result = factorization::greatest_common_divisor(result, *it);
                    return result;
                }

                /** @brief Computes the least common multiplier (LCM) for two numbers.
                 *  @todo Check for negative values.
                 */
                template <typename t_value_type>
                static t_value_type least_common_multiplier(t_value_type a, t_value_type b) noexcept
                {
                    return a > b ?
                        ((a / factorization::greatest_common_divisor(a, b)) * b) :
                        ((b / factorization::greatest_common_divisor(a, b)) * a);
                }

                /** @brief Computes the least common multiplier (LCM) for a list of numbers.
                 *  @todo Check for negative values.
                 */
                template <typename t_value_type>
                static t_value_type least_common_multiplier(std::initializer_list<t_value_type> a) noexcept
                {
                    auto it = a.begin();
                    if (it == a.end()) return 1;

                    t_value_type result = *it;
                    ++it;
                    for (; it != a.end(); ++it) result = factorization::least_common_multiplier(result, *it);
                    return result;
                }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_FACTORIZATION_HPP_INCLUDED
