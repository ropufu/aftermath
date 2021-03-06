
#ifndef ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED

#include <string> // std::string, std::to_string

namespace ropufu::aftermath
{
    namespace detail
    {   
        /** @brief Handles conversion between \tparam t_enum_type and \c std::string. */
        template <ropufu::enumeration t_enum_type>
        struct enum_parser
        {
            using type = enum_parser<t_enum_type>;
            using enum_type = t_enum_type;
            static constexpr bool is_specialized = false;

            static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

            static bool try_parse(const std::string& from, enum_type& to) noexcept { return false; }
        }; // struct enum_parser

        template <ropufu::enumeration t_enum_type>
        bool try_parse_enum(const std::string& from, t_enum_type& to) noexcept
        {
            return enum_parser<t_enum_type>::try_parse(from, to);
        } // try_parse_enum(...)

        template <typename t_value_type>
        concept does_not_specialize_enum_parser = requires
            {
                // Check that enum parser has not been specialized.
                (enum_parser<t_value_type>::is_specialized == false);
            }; // concept unspecialized_enum_parser
    } // namespace detail
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_ENUM_PARSER_HPP_INCLUDED
