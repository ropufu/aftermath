
#ifndef ROPUFU_AFTERMATH_METADATA_HPP_INCLUDED
#define ROPUFU_AFTERMATH_METADATA_HPP_INCLUDED

#include <cstddef>     // std::nullptr_t
#include <cstdint>     // std::int??_t, std::uint??_t
#include <string_view> // std::string_view

#define ROPUFU_MAKE_METADATA(TYPENAME, QUALIFIED_NAME, IS_FUNDAMENTAL)     \
    template <>                                                            \
    struct metadata<TYPENAME>                                              \
    {                                                                      \
        static constexpr bool is_specialized = true;                       \
        static constexpr bool is_fundamental = IS_FUNDAMENTAL;             \
        static constexpr std::string_view qualified_name = QUALIFIED_NAME; \
    };                                                                     \

namespace ropufu
{
    template <typename t_type>
    struct metadata
    {
        static constexpr bool is_specialized = false;
        static constexpr bool is_fundamental = false;
        static constexpr std::string_view qualified_name = "??";
    }; // struct metadata

    template <typename t_type>
    static constexpr std::string_view qualified_name() noexcept { return metadata<t_type>::qualified_name; }

    ROPUFU_MAKE_METADATA(void, "void", true)
    ROPUFU_MAKE_METADATA(std::nullptr_t, "::std::nullptr_t", true)
    ROPUFU_MAKE_METADATA(bool, "bool", true)
    ROPUFU_MAKE_METADATA(char, "char", true)
    ROPUFU_MAKE_METADATA(signed char, "signed char", true)
    ROPUFU_MAKE_METADATA(unsigned char, "unsigned char", true)
    ROPUFU_MAKE_METADATA(float, "float", true)
    ROPUFU_MAKE_METADATA(double, "double", true)
    ROPUFU_MAKE_METADATA(long double, "long double", true)

    ROPUFU_MAKE_METADATA(std::int16_t, "::std::int16_t", true)
    ROPUFU_MAKE_METADATA(std::int32_t, "::std::int32_t", true)
    ROPUFU_MAKE_METADATA(std::int64_t, "::std::int64_t", true)

    ROPUFU_MAKE_METADATA(std::uint16_t, "::std::uint16_t", true)
    ROPUFU_MAKE_METADATA(std::uint32_t, "::std::uint32_t", true)
    ROPUFU_MAKE_METADATA(std::uint64_t, "::std::uint64_t", true)
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_METADATA_HPP_INCLUDED
