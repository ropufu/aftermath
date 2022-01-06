
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_CONCEPTS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_CONCEPTS_HPP_INCLUDED

#include <concepts> // std::same_as, std::convertible_to

namespace ropufu::aftermath::sequential
{
    template <typename t_stopped_type>
    concept stopped_statistic = std::same_as<t_stopped_type, void> ||
        requires(t_stopped_type& s, std::size_t time)
        {
            typename t_stopped_type::value_type;
            {s(time)} -> std::convertible_to<typename t_stopped_type::value_type>;
        }; // concept stopped_statistic
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_CONCEPTS_HPP_INCLUDED
