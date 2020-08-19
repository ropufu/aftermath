
#ifndef ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include "concepts.hpp"

#include <concepts>    // std::floating_point, std::same_as
#include <map>         // std::map
#include <ranges>      // std::ranges::range
#include <set>         // std::set
#include <string>      // std::string, std::basic_string
#include <type_traits> // std::underlying_type_t
#include <utility>     // std::forward
#include <vector>      // std::vector

namespace ropufu
{
    template <typename t_result_type>
    struct noexcept_json_serializer
    {
        // static bool try_get(const nlohmann::json& j, t_result_type& x) noexcept; // To be specialized.
    }; // struct noexcept_json_serializer

    template <typename t_value_type>
    concept json_ready = ropufu::decayed<t_value_type> &&
        requires(const nlohmann::json& j, t_value_type& x)
        {
            {x = j.template get<t_value_type>()};
        }; // concept json_ready

    template <typename t_value_type>
    concept json_noexcept = json_ready<t_value_type> &&
        requires(const nlohmann::json& j, t_value_type& x)
        {
            // Check if noexcept serializer has been specialized.
            {noexcept_json_serializer<t_value_type>::try_get(j, x)} -> std::same_as<bool>;
            noexcept(noexcept_json_serializer<t_value_type>::try_get(j, x));
        }; // concept json_noexcept

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

        static json_pointer_type try_find(const nlohmann::json& j, const std::string& key) noexcept
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
                    result = j.template get<value_type>();
                    return true;
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
            else if constexpr (ropufu::enumeration<value_type>) // Second clause: non-specialized noexcept_json_serializer enumerations.
            {
                using underlying_type = std::underlying_type_t<value_type>;

                underlying_type x {};
                if (!noexcept_json::try_get(j, x)) return false;
                result = static_cast<value_type>(x);
                return true;
            } // if constexpr (...)
            else if constexpr (ropufu::push_back_container<value_type>) // Third clause: vector-like containers.
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
            else if constexpr (ropufu::emplace_dictionary<value_type> &&
                std::same_as<typename nlohmann::json::object_t::key_type, typename value_type::key_type>) // Fourth clause: map-like containers.
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

            return false;
        } // try_get(...)

        template <ropufu::decayed t_value_type>
        static bool required(const nlohmann::json& j, const std::string& key, t_value_type& result) noexcept
        {
            json_pointer_type result_pointer = noexcept_json::try_find(j, key);
            if (result_pointer == nullptr) return false; // Missing keys not allowed.
            return noexcept_json::try_get(*result_pointer, result);
        } // required(...)

        template <ropufu::decayed t_value_type>
        static bool optional(const nlohmann::json& j, const std::string& key, t_value_type& result) noexcept
        {
            json_pointer_type result_pointer = noexcept_json::try_find(j, key);
            if (result_pointer == nullptr) return true; // Missing keys expected.
            if (result_pointer->type() == json_value_type::null) return true; // Explicit nulls allowed.
            return noexcept_json::try_get(*result_pointer, result);
        } // optional(...)
    }; // struct noexcept_json
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_TRY_JSON_HPP_INCLUDED
