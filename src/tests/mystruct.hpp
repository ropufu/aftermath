
#ifndef ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include <cstddef>  // std::size_t
#include <string>   // std::string, std::to_string

namespace ropufu::aftermath::tests
{
    enum struct mystruct
    {
        maybe = 0,
        perhaps = 1,
        almost_surely = 2,
        surely = 3
    }; // enum struct mystruct

    static constexpr std::size_t mystruct_first_index = 0;
    static constexpr std::size_t mystruct_last_index = 3;
} // namespace ropufu::aftermath::tests

namespace std
{
    std::string to_string(ropufu::aftermath::tests::mystruct x) noexcept
    {
        using enum_type = ropufu::aftermath::tests::mystruct;
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

#endif // ROPUFU_AFTERMATH_TESTS_MYSTRUCT_HPP_INCLUDED