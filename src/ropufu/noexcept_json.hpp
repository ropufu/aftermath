
#ifndef ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "concepts.hpp"
#include "enum_parser.hpp"

#include <array>       // std::array
#include <concepts>    // std::floating_point, std::same_as
#include <cstddef>     // std::size_t
#include <map>         // std::map
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::range
#include <set>         // std::set
#include <string>      // std::string, std::basic_string
#include <string_view> // std::string_view
#include <type_traits> // std::underlying_type_t
#include <utility>     // std::forward
#include <variant>     // std::variant
#include <vector>      // std::vector

namespace ropufu
{
    template <typename t_result_type>
    struct noexcept_json_serializer
    {
        /** To be specialized. */
        // static bool try_get(const nlohmann::json& j, t_result_type& x) noexcept;
    }; // struct noexcept_json_serializer

    template <typename t_value_type>
    concept json_ready = ropufu::decayed<t_value_type> &&
        requires(const nlohmann::json& j, t_value_type& x)
        {
            {x = j.template get<t_value_type>()};
        }; // concept json_ready

    template <typename t_value_type>
    concept json_noexcept = ropufu::json_ready<t_value_type> &&
        requires(const nlohmann::json& j, t_value_type& x)
        {
            // Check if noexcept serializer has been specialized.
            {noexcept_json_serializer<t_value_type>::try_get(j, x)} -> std::same_as<bool>;
            noexcept(noexcept_json_serializer<t_value_type>::try_get(j, x));
        }; // concept json_noexcept

    template <typename t_value_type>
    concept enum_noexcept = ropufu::enumeration<t_value_type> &&
        (!ropufu::aftermath::detail::does_not_specialize_enum_parser<t_value_type>) &&
        requires(const std::string& s, t_value_type& x)
        {
            // Check if enum parser has been specialized.
            {ropufu::aftermath::detail::enum_parser<t_value_type>::try_parse(s, x)} -> std::same_as<bool>;
            noexcept(ropufu::aftermath::detail::enum_parser<t_value_type>::try_parse(s, x));
        }; // concept enum_noexcept

    struct noexcept_json;

    namespace detail
    {
        template <typename t_noexcept_json_type, ropufu::decayed t_first_value_type, ropufu::decayed... t_more_value_types>
        struct noexcept_json_variant_discriminator
        {
            template <typename... t_all_value_types>
            static bool discriminate(const nlohmann::json& j, std::variant<t_all_value_types...>& result) noexcept
            {
                t_first_value_type x {};
                if (t_noexcept_json_type::try_get(j, x))
                {
                    result = x;
                    return true;
                } // if (...)
                return noexcept_json_variant_discriminator<t_noexcept_json_type, t_more_value_types...>::discriminate(j, result);
            } // discriminate(...)
        }; // struct noexcept_json_variant_discriminator

        template <typename t_noexcept_json_type, typename t_value_type>
        struct noexcept_json_variant_discriminator<t_noexcept_json_type, t_value_type>
        {
            /** @brief Base case: only one option type left. */
            template <typename... t_all_value_types>
            static bool discriminate(const nlohmann::json& j, std::variant<t_all_value_types...>& result) noexcept
            {
                t_value_type x {};
                if (t_noexcept_json_type::try_get(j, x))
                {
                    result = x;
                    return true;
                } // if (...)
                return false;
            } // discriminate(...)
        }; // struct noexcept_json_variant_discriminator<...>
    } // namespace detail

    struct noexcept_json
    {
        using json_value_type = nlohmann::detail::value_t;
        using json_pointer_type = const nlohmann::json*;

        template <typename t_input_type>
        static bool try_parse(t_input_type&& i, nlohmann::json& j, bool ignore_comments = false) noexcept
        {
            constexpr bool allow_exceptions = false;
            j = nlohmann::json::parse(std::forward<t_input_type>(i), nullptr, allow_exceptions, ignore_comments);
            return !j.is_discarded();
        } // try_parse(...)

        static json_pointer_type try_find(const nlohmann::json& j, std::string_view key) noexcept
        {
            if (j.is_discarded()) return nullptr;

            const auto search = j.find(key);
            if (search == j.end()) return nullptr;
            
            return &(*search);
        } // try_find(...)

        template <typename t_letter_type>
        static bool try_get(const nlohmann::json& j, std::basic_string<t_letter_type>& result) noexcept
        {
            using value_type = std::basic_string<t_letter_type>;
            switch (j.type())
            {
                case json_value_type::string:
                    result = j.template get<value_type>();
                    return true;
                default: return false;
            } // switch (...)
        } // try_get(...)

        /** Tries to serialize a JSON array as \c std::array. Fails if sizes do not match. */
        template <ropufu::decayed t_value_type, std::size_t t_size>
        static bool try_get(const nlohmann::json& j, std::array<t_value_type, t_size>& result) noexcept
        {
            using value_type = t_value_type;

            std::vector<value_type> x {};
            if (!noexcept_json::try_get(j, x)) return false;
            if (x.size() != t_size) return false;

            auto result_it = result.begin();
            auto vector_it = x.cbegin();
            for (std::size_t i = 0; i < t_size; ++i)
            {
                *result_it = *vector_it;
                ++result_it;
                ++vector_it;
            } // for (...)

            return true;
        } // try_get(...)

        /** Tries to serialize a JSON array as \c std::set. Fails if source contains duplicate values. */
        template <ropufu::decayed t_value_type>
        static bool try_get(const nlohmann::json& j, std::set<t_value_type>& result) noexcept
        {
            using value_type = t_value_type;
            switch (j.type())
            {
                case json_value_type::array:
                    result.clear();
                    for (const nlohmann::json& k : j)
                    {
                        value_type x {};
                        if (!noexcept_json::try_get(k, x)) return false;
                        if (!result.insert(x).second) return false; // Duplicate values in source not allowed.
                    } // for (...)
                    return true;
                default: return false;
            } // switch (...)
        } // try_get(...)

        /** Tries to serialize an optional JSON value. */
        template <ropufu::decayed t_value_type>
        static bool try_get(const nlohmann::json& j, std::optional<t_value_type>& result) noexcept
        {
            using value_type = t_value_type;
            switch (j.type())
            {
                case json_value_type::discarded: return false;
                case json_value_type::null:
                    result = std::nullopt;
                    return true;
                default:
                    value_type x {};
                    if (!noexcept_json::try_get(j, x)) return false;
                    result = x;
                    return true;
            } // switch (...)
        } // try_get(...)

        /** Tries to serialize a JSON value that can be one of several types.
         *  The first that works will be selected.
         */
        template <ropufu::decayed... t_value_types>
        static bool try_get(const nlohmann::json& j, std::variant<t_value_types...>& result) noexcept
        {
            return detail::noexcept_json_variant_discriminator<noexcept_json, t_value_types...>::discriminate(j, result);
        } // try_get(...)

        static bool try_get(const nlohmann::json& j, bool& result) noexcept
        {
            using value_type = bool;
            switch (j.type())
            {
                case json_value_type::boolean:
                    result = j.template get<value_type>();
                    return true;
                // case json_value_type::number_unsigned: // Non-negative integer.
                //     std::size_t x = j.template get<std::size_t>();
                //     if (x == 0) { result = false; return true; }
                //     if (x == 1) { result = true; return true; }
                //     return false;
                default: return false;
            } // switch (...)
        } // try_get(...)

        template <ropufu::decayed t_value_type>
            requires ropufu::integer<t_value_type>
        static bool try_get(const nlohmann::json& j, t_value_type& result) noexcept
        {
            using value_type = t_value_type;
            switch (j.type())
            {
                case json_value_type::number_integer: // Negative integer.
                    if constexpr (!ropufu::numeric_signed<value_type>) return false;
                    else
                    {
                        result = j.template get<value_type>();
                        return true;
                    } // if constexpr
                case json_value_type::number_unsigned: // Non-negative integer.
                    result = j.template get<value_type>();
                    return true;
                default: return false;
            } // switch (...)
        } // try_get(...)

        template <ropufu::decayed t_value_type>
            requires std::floating_point<t_value_type>
        static bool try_get(const nlohmann::json& j, t_value_type& result) noexcept
        {
            using value_type = t_value_type;
            switch (j.type())
            {
                case json_value_type::number_integer: // Negative integer.
                case json_value_type::number_unsigned: // Non-negative integer.
                case json_value_type::number_float: // Floating-point number.
                    result = j.template get<value_type>();
                    return true;
                default: return false;
            } // switch (...)
        } // try_get(...)

        template <ropufu::decayed t_value_type>
        static bool try_get(const nlohmann::json& j, t_value_type& result) noexcept
        {
            using value_type = t_value_type;
            using serializer_type = noexcept_json_serializer<t_value_type>;

            if (j.is_discarded()) return false;

            if constexpr (ropufu::json_noexcept<value_type>) // First clause: type is already noexcept-ready (specialized noexcept_json_serializer).
            {
                return serializer_type::try_get(j, result);
            } // if constexpr (...)
            else if constexpr (ropufu::enum_noexcept<value_type>) // Second clause: type is a specialized enum_parser-ready enumeration.
            {
                std::string x {};
                if (!noexcept_json::try_get(j, x)) return false;
                if (!ropufu::aftermath::detail::try_parse_enum(x, result)) return false;
                return true;
            } // if constexpr (...)
            else if constexpr (ropufu::enumeration<value_type>) // Third clause: non-specialized noexcept_json_serializer enumerations.
            {
                using underlying_type = std::underlying_type_t<value_type>;

                underlying_type x {};
                if (!noexcept_json::try_get(j, x)) return false;
                result = static_cast<value_type>(x);
                return true;
            } // if constexpr (...)
            else if constexpr (ropufu::push_back_container<value_type>) // Fourth clause: vector-like containers.
            {
                using element_type = typename value_type::value_type;
                switch (j.type())
                {
                    case json_value_type::array:
                        result.clear();
                        for (const nlohmann::json& k : j)
                        {
                            element_type x {};
                            if (!noexcept_json::try_get(k, x)) return false;
                            result.push_back(x);
                        } // for (...)
                        return true;
                    default: return false;
                } // switch (...)
            } // if constexpr (...)
            else if constexpr (ropufu::emplace_dictionary_with_key<value_type, typename nlohmann::json::object_t::key_type>) // Fifth clause: map-like containers.
            {
                using element_type = typename value_type::mapped_type;
                switch (j.type())
                {
                    case json_value_type::object:
                        result.clear();
                        for (auto it = j.begin(); it != j.end(); ++it)
                        {
                            const auto& key = it.key();
                            const nlohmann::json& k = it.value();

                            element_type x {};
                            if (!noexcept_json::try_get(k, x)) return false;
                            if (!result.emplace(key, x).second) return false; // Duplicate keys in source not allowed.
                        } // for (...)
                        return true;
                    default: return false;
                } // switch (...)
            } // if constexpr (...)
            else
            {
                return false;
            } // if constexpr (...)
        } // try_get(...)

        template <ropufu::decayed t_value_type>
        static bool required(const nlohmann::json& j, std::string_view key, t_value_type& result) noexcept
        {
            json_pointer_type result_pointer = noexcept_json::try_find(j, key);
            if (result_pointer == nullptr) return false; // Missing keys not allowed.
            return noexcept_json::try_get(*result_pointer, result);
        } // required(...)

        template <ropufu::decayed t_value_type>
        static bool optional(const nlohmann::json& j, std::string_view key, t_value_type& result) noexcept
        {
            json_pointer_type result_pointer = noexcept_json::try_find(j, key);
            if (result_pointer == nullptr) return true; // Missing keys expected.
            if (result_pointer->type() == json_value_type::null) return true; // Explicit nulls allowed.
            return noexcept_json::try_get(*result_pointer, result);
        } // optional(...)
    }; // struct noexcept_json
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED
