
#ifndef ROPUFU_AFTERMATH_PROBABILITY_CONCEPTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_CONCEPTS_HPP_INCLUDED

#include <concepts> // std::assignable_from, std::same_as
#include <limits>   // std::numeric_limits
#include <utility>  // std::declval
#include "../P0870.hpp" // ropufu::is_narrowing_convertible_v

namespace ropufu
{
    template <typename t_distribution_type>
    concept distribution = requires(const t_distribution_type& d)
    {
        typename t_distribution_type::value_type;
        typename t_distribution_type::probability_type;
        typename t_distribution_type::expectation_type;

        requires !ropufu::is_narrowing_convertible_v<
            decltype(std::declval<typename t_distribution_type::probability_type>() * std::declval<typename t_distribution_type::value_type>()),
            typename t_distribution_type::expectation_type>;

        {d.expected_value()} -> std::same_as<typename t_distribution_type::expectation_type>;
        {d.variance()} -> std::same_as<typename t_distribution_type::expectation_type>;
        {d.standard_deviation()} -> std::same_as<typename t_distribution_type::expectation_type>;

        {d.mean()} -> std::same_as<typename t_distribution_type::expectation_type>;
        {d.stddev()} -> std::same_as<typename t_distribution_type::expectation_type>;

        {d.cdf(std::declval<typename t_distribution_type::value_type>())}
            -> std::same_as<typename t_distribution_type::probability_type>;
    }; // concept distribution
} // namespace ropufu

namespace ropufu::aftermath::probability
{
    template <typename t_numeric_type>
    inline constexpr t_numeric_type default_quantile_tolerance = 10 * std::numeric_limits<t_numeric_type>::epsilon();
    
    /** Indicates if the distribution is continuous. */
    template <typename t_distribution_type>
    struct is_continuous
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct is_continuous

    /** Indicates if the distribution is continuous. */
    template <typename t_distribution_type>
    inline constexpr bool is_continuous_v = is_continuous<t_distribution_type>::value;

    /** Indicates if the distribution is discrete. */
    template <typename t_distribution_type>
    struct is_discrete
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct is_discrete

    /** Indicates if the distribution is discrete. */
    template <typename t_distribution_type>
    inline constexpr bool is_discrete_v = is_discrete<t_distribution_type>::value;

    /** Indicates if the distribution has an infinite left tail. */
    template <typename t_distribution_type>
    struct has_left_tail
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct has_left_tail

    /** Indicates if the distribution has an infinite left tail. */
    template <typename t_distribution_type>
    inline constexpr bool has_left_tail_v = has_left_tail<t_distribution_type>::value;

    /** Indicates if the distribution has an infinite right tail. */
    template <typename t_distribution_type>
    struct has_right_tail
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct has_right_tail

    /** Indicates if the distribution has an infinite right tail. */
    template <typename t_distribution_type>
    inline constexpr bool has_right_tail_v = has_right_tail<t_distribution_type>::value;

    /** Indicates if the distribution has bounded support. */
    template <typename t_distribution_type>
    inline constexpr bool has_bounded_support_v = (!has_left_tail_v<t_distribution_type>) && (!has_right_tail_v<t_distribution_type>);

    namespace detail
    {
        template <ropufu::distribution t_distribution_type>
        struct i_distribution {};
    } // namespace detail

    /** @brief Helper class to ensure the \c distribution concept requirements are met. */
    template <typename t_distribution_type>
    struct distribution_base
    {
        distribution_base() noexcept { detail::i_distribution<t_distribution_type>{}; }
    }; // struct distribution_base
} // namespace ropufu::aftermath::probability

#endif // ROPUFU_AFTERMATH_PROBABILITY_CONCEPTS_HPP_INCLUDED
