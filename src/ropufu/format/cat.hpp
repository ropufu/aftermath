
#ifndef ROPUFU_AFTERMATH_FORMAT_CAT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_CAT_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <sstream> // std::ostringstream
#include <string>  // std::string
#include <utility> // std::forward

namespace ropufu::aftermath::format
{
    namespace detail
    {
        template <typename t_first_type>
        static void dog(std::ostringstream& stream, t_first_type&& first)
        {
            stream << first;
        } // cat(...)

        template <typename t_first_type, typename... t_arg_types>
        static void dog(std::ostringstream& stream, t_first_type&& first, t_arg_types&&... args)
        {
            stream << first;
            dog(stream, std::forward<t_arg_types>(args)...);
        } // cat(...)
    } // namespace detail

    /** @brief Handles variable naming for .mat files. */
    template <typename... t_arg_types>
    static std::string cat(t_arg_types&&... args)
    {
        std::ostringstream stream {};
        detail::dog(stream, std::forward<t_arg_types>(args)...);
        return stream.str();
    } // cat(...)

    /** A version of snake-case where all non-digit or non-latin letters are replaced with underscores. */
    [[maybe_unused]]
    static std::string snake(const std::string& value) noexcept
    {
        std::string result = value;
        for (char& c : result)
        {
            if (c >= '0' && c <= '9') continue; // Digit.
            if (c >= 'a' && c <= 'z') continue; // Lowercase latin.
            if (c >= 'A' && c <= 'Z') continue; // Uppercase latin.
            c = '_';
        } // for (...)
        return result;
    } // mat_name(...)
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_CAT_HPP_INCLUDED
