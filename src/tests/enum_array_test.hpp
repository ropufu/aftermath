
#include <nlohmann/json.hpp>

#include "../ropufu/enum_array.hpp"
#include "core.hpp"

#include <array>    // std::array
#include <cstddef>  // std::size_t
#include <cstdint>  // std::int32_t
#include <string>   // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::test_aftermath
{
    enum struct funny
    {
        maybe = 0,
        perhaps = 1,
        almost_surely = 2,
        surely = 3
    }; // enum struct funny
} // namespace ropufu::test_aftermath

namespace std
{
    std::string to_string(ropufu::test_aftermath::funny x) noexcept
    {
        using enum_type = ropufu::test_aftermath::funny;
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
    template <>
    struct enum_array_keys<ropufu::test_aftermath::funny>
    {
        using underlying_type = std::underlying_type_t<ropufu::test_aftermath::funny>;
        static constexpr underlying_type first_index = 1;
        static constexpr underlying_type past_the_last_index = 4;
    }; // struct enum_array_keys<...>

    template <>
    struct enum_parser<ropufu::test_aftermath::funny>
    {
        using enum_type = ropufu::test_aftermath::funny;

        static std::string to_string(const enum_type& from) noexcept
        {
            return std::to_string(from);
        } // to_string(...)

        static bool try_parse(const std::string& from, enum_type& to) noexcept
        {
            if (from == "maybe") { to = enum_type::maybe; return true; }
            if (from == "perhaps") { to = enum_type::perhaps; return true; }
            if (from == "almost surely") { to = enum_type::almost_surely; return true; }
            if (from == "surely") { to = enum_type::surely; return true; }
            return false;
        } // try_parse(...)
    }; // struct enum_parser<...>
} // namespace ropufu::aftermath::detail

namespace ropufu::test_aftermath
{
    struct enum_array_test
    {
        using enum_type = ropufu::test_aftermath::funny;
        using underlying_type = std::underlying_type_t<enum_type>;

        template <typename t_type>
        using enum_array_t = ropufu::aftermath::enum_array<enum_type, t_type>;

        template <typename t_numeric_type>
        static bool basic_test() noexcept
        {
            enum_array_t<t_numeric_type> tested { };

            t_numeric_type i = 0;
            for (auto x : tested) x.value() = ++i;
            
            if (!test_json_round_trip(tested)) return false;
            return true;
        } // basic_test(...)

        static bool basic_test_bool() noexcept
        {
            enum_array_t<bool> tested { enum_type::almost_surely, enum_type::surely };

            tested.set(enum_type::maybe);
            tested.set(enum_type::perhaps);
            tested.unset(enum_type::almost_surely);

            underlying_type aggregate = 0;
            for (enum_type x : tested) aggregate += static_cast<underlying_type>(x);
            
            if (!test_json_round_trip(tested)) return false;
            return true;
        } // basic_test_bool(...)

        static bool basic_test_void() noexcept
        {
            enum_array_t<void> tested { };

            underlying_type aggregate = 0;
            for (enum_type x : tested) aggregate += static_cast<underlying_type>(x);
            
            if (!test_json_round_trip(tested)) return false;
            return true;
        } // basic_test_void(...)
    }; // struct enum_array_test
} // namespace ropufu::test_aftermath
