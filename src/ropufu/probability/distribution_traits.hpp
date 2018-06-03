
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DISTRIBUTION_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DISTRIBUTION_TRAITS_HPP_INCLUDED

namespace ropufu::aftermath::probability
{
    /** Indicates if the distribution is continuous. */
    template <typename t_distribution_type>
    struct is_continuous
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct is_continuous

    template <typename t_distribution_type>
    inline constexpr bool is_continuous_v = is_continuous<t_distribution_type>::value;

    /** Indicates if the distribution is discrete. */
    template <typename t_distribution_type>
    struct is_discrete
    {
        using distribution_type = t_distribution_type;
        static constexpr bool value = false;
    }; // struct is_discrete

    template <typename t_distribution_type>
    inline constexpr bool is_discrete_v = is_discrete<t_distribution_type>::value;
} // namespace ropufu::aftermath::probability

#endif // ROPUFU_AFTERMATH_PROBABILITY_DISTRIBUTION_TRAITS_HPP_INCLUDED
