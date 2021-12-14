
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../simple_vector.hpp"
#include "timed_transform.hpp"
#include "window_limited_statistic.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME window_limited_cusum<t_value_type, t_container_type, t_transform_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE                                                     \
    template <std::totally_ordered t_value_type,                                          \
    std::ranges::random_access_range t_container_type,                                    \
    ropufu::aftermath::sequential::timed_transform<t_value_type> t_transform_type>        \
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type> \


namespace ropufu::aftermath::sequential
{
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = simple_vector<t_value_type>,
        ropufu::aftermath::sequential::timed_transform<t_value_type> t_transform_type = identity_transform<t_value_type>>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct window_limited_cusum;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    /** Window-limited CUSUM chart. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct window_limited_cusum : public window_limited_statistic<t_value_type, t_container_type, t_transform_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = window_limited_statistic<t_value_type, t_container_type, t_transform_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;
        using transform_type = t_transform_type;

        /** Names the statistic type. */
        constexpr std::string_view name() const noexcept override
        {
            return "Window-limited CUSUM";
        } // name(...)

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

        using base_type::base_type;

        bool operator ==(const type& other) const noexcept
        {
            return this->equals(other);
        } // operator ==(...)

        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            x.serialize_core(j);
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <window_limited_cusum> failed: " + j.dump());
        } // from_json(...)
#endif

    protected:
        /** Occurs when the most recent observation has been added to the history.
         *  @param history If L is the window size, then the history at time n contains observations @ times:
         *  ... --- (n - L + 1) ---  n --- (n - 1) --- (n - 2) --- ...
         *            oldest       newest
         *  @param newest_index Points to the newest item in \param history.
         */
        value_type on_history_updated(const container_type& history, std::size_t newest_index) noexcept override
        {
            value_type sum = 0;
            value_type max = 0;
            for (std::size_t i = newest_index; i < history.size(); ++i)
            {
                sum += history[i];
                if (sum > max) max = sum;
            } // for (...)
            
            for (std::size_t i = 0; i < newest_index; ++i)
            {
                sum += history[i];
                if (sum > max) max = sum;
            } // for (...)
            return max;
        } // on_history_updated(...)

        constexpr void on_reset() noexcept override
        {
        } // on_reset(...)
    }; // struct window_limited_cusum
} // namespace ropufu::aftermath::sequential

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct noexcept_json_serializer<ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME>
    {
        using result_type = ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!x.try_deserialize_core(j)) return false;

            std::size_t window_size;
            if (!noexcept_json::required(j, result_type::jstr_window_size, window_size)) return false;
            
            x.initialize(window_size);
            if (x.error_message().has_value()) return false;
            
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

namespace std
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct hash<ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME>
    {
        using argument_type = ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            result_type result = x.get_hash();
            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
