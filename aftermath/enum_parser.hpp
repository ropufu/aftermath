
#ifndef ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED

#include <string>      // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::aftermath
{
    namespace detail
    {
        /** @brief Handles conversion between differenct types. */
        template <typename t_enum_type>
        struct enum_parser
        {
            using type = enum_parser<t_enum_type>;
            using enum_type = t_enum_type;
            using underlying_type = std::underlying_type_t<t_enum_type>;

            static std::string to_string(const enum_type& from) noexcept
            {
                return std::to_string(from);
                //return std::to_string(static_cast<underlying_type>(from));
            } // to_string(...)

            static bool try_parse(const std::string& from, enum_type& to) noexcept { return false; }
        }; // struct converter
    } // namespace detail
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED
