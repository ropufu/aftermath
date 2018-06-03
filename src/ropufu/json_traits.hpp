
#ifndef ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED

#include <string> // std::string, std::to_string

namespace ropufu::aftermath
{
    /** @brief Handles various taks related to json (de-)serialization. */
    template <typename t_serialized_type>
    struct json_traits
    {
        using type = json_traits<t_serialized_type>;
        using serialized_type = t_serialized_type;

        static void warning(std::string&& /*message*/) noexcept { }
    }; // struct json_traits
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED
