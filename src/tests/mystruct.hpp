
#ifndef ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

#include "../ropufu/enum_parser.hpp"
#include "../ropufu/enum_array.hpp"

#include <cstddef>  // std::size_t
#include <string>   // std::string, std::to_string

namespace ropufu::tests
{
    enum struct mystruct
    {
        maybe = 0,
        perhaps = 1,
        almost_surely = 2,
        surely = 3
    }; // enum struct mystruct

    // static constexpr std::size_t mystruct_first_index = 0;
    // static constexpr std::size_t mystruct_last_index = 3;

#ifndef ROPUFU_NO_JSON
    void to_json(nlohmann::json& j, const mystruct& x) noexcept;

    void from_json(const nlohmann::json& j, mystruct& x);
#endif
} // namespace ropufu::tests

namespace std
{
    std::string to_string(ropufu::tests::mystruct x) noexcept
    {
        using enum_type = ropufu::tests::mystruct;
        switch (x)
        {
            case enum_type::maybe: return "maybe";
            case enum_type::perhaps: return "perhaps";
            case enum_type::almost_surely: return "almost surely";
            case enum_type::surely: return "surely";
            default: return std::to_string(static_cast<std::size_t>(x));
        } // switch (...)
    } // to_string(...)
} // namespace std

namespace ropufu::aftermath::detail
{
    /** Mark \c mystruct as suitable for \c enum_array storage. */
    template <>
    struct enum_array_keys<ropufu::tests::mystruct>
    {
        using underlying_type = std::underlying_type_t<ropufu::tests::mystruct>;
        static constexpr underlying_type first_index = 0;
        static constexpr underlying_type past_the_last_index = 4;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::tests::mystruct>
    {
        using enum_type = ropufu::tests::mystruct;

        static std::string to_string(const enum_type& from) noexcept { return std::to_string(from); }

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "maybe") { to = enum_type::maybe; return true; }
            if (from == "perhaps") { to = enum_type::perhaps; return true; }
            if (from == "almost surely" || from == "as") { to = enum_type::almost_surely; return true; }
            if (from == "surely") { to = enum_type::surely; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

#ifndef ROPUFU_NO_JSON
namespace ropufu::tests
{
    void to_json(nlohmann::json& j, const mystruct& x) noexcept
    {
        j = aftermath::detail::enum_parser<mystruct>::to_string(x);
    } // to_json(...)

    void from_json(const nlohmann::json& j, mystruct& x)
    {
        if (!j.is_string()) throw std::runtime_error("Parsing <mystruct> failed: " + j.dump());
        std::string s = j.get<std::string>();
        if (!aftermath::detail::try_parse_enum(s, x)) throw std::runtime_error("<mystruct> not recognized: " + j.dump());
    } // from_json(...)
} // namespace ropufu::tests
#endif

#endif // ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED