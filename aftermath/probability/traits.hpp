
#ifndef ROPUFU_AFTERMATH_PROBABILITY_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_TRAITS_HPP_INCLUDED

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            /** Indicates if the distribution is continuous. */
            template <typename t_distribution_type>
            struct is_continuous
            {
                using distribution_type = t_distribution_type;
                static constexpr bool value = false;
            }; // struct is_continuous

            /** Indicates if the distribution is discrete. */
            template <typename t_distribution_type>
            struct is_discrete
            {
                using distribution_type = t_distribution_type;
                static constexpr bool value = false;
            }; // struct is_discrete
        } // namespace probability
    } // namespace aftermath
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_PROBABILITY_TRAITS_HPP_INCLUDED
