
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED

namespace ropufu::aftermath::sequential
{
    /** Abstract class for observing a discrete process. */
    template <typename t_observation_value_type, typename t_statistic_value_type = t_observation_value_type>
    struct statistic
    {
        using type = statistic<t_observation_value_type, t_statistic_value_type>;
        using observation_value_type = t_observation_value_type;
        using statistic_value_type = t_statistic_value_type;

        statistic() noexcept = default;
        
        virtual ~statistic() noexcept = default;

        /** The underlying process has been cleared. */
        virtual void reset() noexcept = 0;

        /** Observe a single value and returns the updated statistic. */
        virtual statistic_value_type observe(const observation_value_type& /*value*/) noexcept = 0;
    }; // struct statistic
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED
