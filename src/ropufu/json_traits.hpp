
#ifndef ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include <string>       // std::string, std::to_string
#include <system_error> // std::error_code, std::errc, std::make_error_code
#include <type_traits>  // std::decay_t, ...
#include <utility>      // std::declval

namespace ropufu::aftermath
{
    namespace detail
    {
        template <typename, typename = void> struct has_nlohmann_error_code_ctor : public std::false_type { };
        template <typename t_type> struct has_nlohmann_error_code_ctor<t_type, std::void_t<
            decltype( t_type(std::declval<const nlohmann::json&>(), std::declval<std::error_code&>()) )
            >> : public std::true_type { };

        template <typename t_type> inline constexpr bool has_nlohmann_error_code_ctor_v = has_nlohmann_error_code_ctor<t_type>::value;
    } // namespace detail

    /** @brief Handles various taks related to json (de-)serialization. */
    template <typename t_serialized_type>
    struct json_traits
    {
        using type = json_traits<t_serialized_type>;
        using serialized_type = t_serialized_type;

        static void warning(std::string&& /*message*/) noexcept { }
    }; // struct json_traits

    template <typename, typename = void> struct has_noexcept_json_ctr : public std::false_type { };

    template <typename t_type> struct has_noexcept_json_ctr<t_type, std::void_t<
            decltype( t_type::t_type(std::declval<nlohmann::json>(), std::declval<std::error_code>()) )
        >> : public std::true_type { };

    template <typename t_type>
    inline constexpr bool has_noexcept_json_ctr_v = has_noexcept_json_ctr<t_type>::value;

    struct noexcept_json
    {
        using type = noexcept_json;

        template <typename t_value_type>
        static void required(const nlohmann::json& j, const std::string& key, t_value_type& result, std::error_code& ec) noexcept
        {
            type::parse(j, key, result, ec, false);
        } // required(...)

        template <typename t_value_type>
        static void optional(const nlohmann::json& j, const std::string& key, t_value_type& result, std::error_code& ec) noexcept
        {
            type::parse(j, key, result, ec, true);
        } // optional(...)

        /** @brief Tries to parse \c j.
         *  @todo Check behavior for \c std::basic_string variations.
         *  @param ec Set to \c std::errc::wrong_protocol_type if the \c json representation is ill-formatted.
         *  @param ec Set to \c std::errc::operation_canceled if any other error was encountered.
         */
        template <typename t_value_type>
        static void as(const nlohmann::json& j, t_value_type& result, std::error_code& ec) noexcept
        {
            type::interpret(j, result, ec);
        } // as(...)

    private:
        /** @brief Tries to parse \c j[key].
         *  @todo Check behavior for \c std::basic_string variations.
         *  @param ec Set to \c std::errc::address_not_available if the key is required but missing.
         *  @param ec Set to \c std::errc::wrong_protocol_type if the \c json representation of \c j[key] is ill-formatted.
         *  @param ec Set to \c std::errc::operation_canceled if any other error was encountered.
         */
        template <typename t_value_type>
        static void parse(const nlohmann::json& j, const std::string& key, t_value_type& result, std::error_code& ec, bool is_optional) noexcept
        {
            using decayed_type = std::decay_t<t_value_type>;
            if (!type::exists(j, key, ec, is_optional)) return;
            type::interpret(j[key], result, ec); // We made sure that the key exists.
        } // parse(...)

        /** @brief Checks if the \p key exists in \p j.
         *  @param ec Set to \c std::errc::address_not_available if the key is required but missing.
         */
        static bool exists(const nlohmann::json& j, const std::string& key, std::error_code& ec, bool is_optional) noexcept
        {
            if (j.count(key) == 0) // Check if the key is missing.
            {
                if (is_optional) return false;
                ec = std::make_error_code(std::errc::address_not_available);
                return false;
            } // if (...)
            return true;
        } // exists(...)

        /** @brief Tries to parse \c j.
         *  @todo Check behavior for \c std::basic_string variations.
         *  @param ec Set to \c std::errc::wrong_protocol_type if the \c json representation is ill-formatted.
         *  @param ec Set to \c std::errc::operation_canceled if any other error was encountered.
         */
        template <typename t_value_type>
        static void interpret(const nlohmann::json& j, t_value_type& result, std::error_code& ec) noexcept
        {
            using decayed_type = std::decay_t<t_value_type>;
            if constexpr (std::is_same_v<decayed_type, bool>) // ~~ Booleans ~~
            {
                if (!j.is_boolean()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }
                result = j.get<decayed_type>();
            } // if constexpr(...)
            else if constexpr (std::is_floating_point_v<decayed_type>) // ~~ Floating point numbers ~~
            {
                if (!j.is_number()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }
                result = j.get<decayed_type>();
            } // if constexpr(...)
            else if constexpr (std::is_integral_v<decayed_type>) // ~~ Integers ~~
            {
                if (!j.is_number_integer()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }
                result = j.get<decayed_type>();
            } // if constexpr(...)
            else if constexpr (std::is_convertible_v<decayed_type, std::string>) // ~~ Strings ~~
            {
                if (!j.is_string()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }
                result = j.get<decayed_type>();
            } // if constexpr(...)
            else if constexpr (detail::has_nlohmann_error_code_ctor_v<decayed_type>) // ~~ Types quietly constructible from json ~~
            {
                result = t_value_type(j, ec);
            } // if constexpr(...)
            else // ~~ Fallback ~~
            {
                try
                {
                    result = j.get<decayed_type>();
                } // try
                catch (...)
                {
                    ec = std::make_error_code(std::errc::operation_canceled);
                    return;
                } // catch (...)
            } // if constexpr(...)
        } // interpret(...)
        
        /** @brief Tries to parse \c j as an array.
         *  @param ec Set to \c std::errc::wrong_protocol_type if \c j is not an array.
         *  @param ec Set to \c std::errc::invalid_argument if there is a size mismatch in arrays.
         */
        template <typename t_value_type, typename t_size_type, t_size_type t_size>
        static void interpret(const nlohmann::json& j, std::array<t_value_type, t_size>& result, std::error_code& ec) noexcept
        {
            if (!j.is_array()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }

            std::size_t count = 0;
            for (const nlohmann::json& item : j)
            {
                if (count == t_size)
                {
                    ec = std::invalid_argument(std::errc::invalid_argument);
                    return;
                } // if (...)

                t_value_type item_value {};
                type::interpret(item, item_value, ec); // Hello, recursion!
                if (ec) return;

                result[count] = item_value;
                ++count;
            } // for (...)

            if (count != t_size) ec = std::invalid_argument(std::errc::invalid_argument);
        } // interpret(...)
        
        /** @brief Tries to parse \c j as a vector.
         *  @param ec Set to \c std::errc::wrong_protocol_type if \c j is not a vector.
         */
        template <typename t_value_type>
        static void interpret(const nlohmann::json& j, std::vector<t_value_type>& result, std::error_code& ec) noexcept
        {
            if (!j.is_array()) { ec = std::make_error_code(std::errc::wrong_protocol_type); return; }

            result.clear();
            result.reserve(j.size());
            for (const nlohmann::json& item : j)
            {
                t_value_type item_value {};
                type::interpret(item, item_value, ec); // Hello, recursion!
                if (ec) return;

                result.push_back(item_value);
            } // for (...)
            result.shrink_to_fit();
        } // interpret(...)
    }; // struct noexcept_json
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_JSON_TRAITS_HPP_INCLUDED
