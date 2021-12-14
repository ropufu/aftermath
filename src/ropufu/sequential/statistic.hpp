
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED

#include <concepts> // std::same_as
#include <cstddef>  // std::size_t
#include <ranges>   // std::ranges::...

namespace ropufu::aftermath::sequential
{
    /** Abstract class for observing a discrete process.
     *  @todo Restrict t_statistic_container_type to be either
     *    -- std::ranges::random_access_range with correct value_type; or
     *    -- void if t_statistic_value_type is void.
     */
    template <typename t_observation_value_type,
        std::ranges::random_access_range t_observation_container_type,
        typename t_statistic_value_type = t_observation_value_type,
        typename t_statistic_container_type = t_observation_container_type>
        requires std::same_as<std::ranges::range_value_t<t_observation_container_type>, t_observation_value_type>
    struct statistic
    {
        using type = statistic<t_observation_value_type, t_observation_container_type, t_statistic_value_type, t_statistic_container_type>;
        using observation_value_type = t_observation_value_type;
        using observation_container_type = t_observation_container_type;
        using statistic_value_type = t_statistic_value_type;
        using statistic_container_type = t_statistic_container_type;

        statistic() noexcept = default;
        
        virtual ~statistic() noexcept = default;

        /** The underlying process has been cleared. */
        virtual void reset() noexcept = 0;

        /** Observe a single value and returns the updated statistic. */
        virtual statistic_value_type observe(const observation_value_type& /*value*/) noexcept = 0;

        /** Observe a block of values and returns the updated statistics. */
        virtual statistic_container_type observe(const observation_container_type& /*values*/) noexcept = 0;
    }; // struct statistic

    template <typename t_observation_value_type, std::ranges::random_access_range t_observation_container_type, typename t_statistic_value_type>
        requires std::same_as<std::ranges::range_value_t<t_observation_container_type>, t_observation_value_type>
    struct statistic<t_observation_value_type, t_observation_container_type, t_statistic_value_type, void>
    {
        using type = statistic<t_observation_value_type, t_observation_container_type, t_statistic_value_type, void>;
        using observation_value_type = t_observation_value_type;
        using observation_container_type = t_observation_container_type;
        using statistic_value_type = t_statistic_value_type;
        using statistic_container_type = void;

        statistic() noexcept = default;
        
        virtual ~statistic() noexcept = default;

        /** The underlying process has been cleared. */
        virtual void reset() noexcept = 0;

        /** Observe a single value and returns the updated statistic. */
        virtual statistic_value_type observe(const observation_value_type& /*value*/) noexcept = 0;

        /** Observe a block of values and returns the updated statistics. */
        virtual statistic_container_type observe(const observation_container_type& values) noexcept
        {
            for (const observation_value_type& x : values) this->observe(x);
        } // observe(...)
    }; // struct statistic

    template <typename t_observation_value_type, std::ranges::random_access_range t_observation_container_type>
        requires std::same_as<std::ranges::range_value_t<t_observation_container_type>, t_observation_value_type>
    struct statistic<t_observation_value_type, t_observation_container_type, t_observation_value_type, t_observation_container_type>
    {
        using type = statistic<t_observation_value_type, t_observation_container_type, t_observation_value_type, t_observation_container_type>;
        using observation_value_type = t_observation_value_type;
        using observation_container_type = t_observation_container_type;
        using statistic_value_type = t_observation_value_type;
        using statistic_container_type = t_observation_container_type;

        statistic() noexcept = default;
        
        virtual ~statistic() noexcept = default;

        /** The underlying process has been cleared. */
        virtual void reset() noexcept = 0;

        /** Observe a single value and returns the updated statistic. */
        virtual statistic_value_type observe(const observation_value_type& /*value*/) noexcept = 0;

        /** Observe a block of values and returns the updated statistics. */
        virtual statistic_container_type observe(const observation_container_type& values) noexcept
        {
            statistic_container_type statistics = values;
            for (std::size_t i = 0; i < values.size(); ++i)
                statistics[i] = this->observe(values[i]);
            return statistics;
        } // observe(...)
    }; // struct statistic
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_STATISTIC_HPP_INCLUDED
