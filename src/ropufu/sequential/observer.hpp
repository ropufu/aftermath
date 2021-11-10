
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_OBSERVER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_OBSERVER_HPP_INCLUDED

#include <concepts> // std::same_as
#include <cstddef>  // std::size_t
#include <ranges>   // std::ranges::...

namespace ropufu::aftermath::sequential
{
    template <typename t_value_type, std::ranges::random_access_range t_container_type>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct observer
    {
        using type = observer<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

        /** The underlying process has been cleared. */
        virtual void reset() noexcept = 0;

        /** Observe a single value. */
        virtual void observe(value_type /*value*/) noexcept = 0;

        /** Observe a block of values. */
        virtual void observe(const container_type& /*values*/) noexcept = 0;

        virtual ~observer() noexcept { }
    }; // struct observer
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_OBSERVER_HPP_INCLUDED
