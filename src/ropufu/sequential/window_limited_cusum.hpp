
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "timed_transform.hpp"
#include "window_limited_statistic.hpp"

#include <concepts>    // std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <stdexcept>   // std::runtime_error
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME window_limited_cusum<t_observation_value_type, t_statistic_value_type, t_transform_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE                                                             \
    template <std::totally_ordered t_observation_value_type,                                      \
        std::totally_ordered t_statistic_value_type,                                              \
        ropufu::aftermath::sequential::timed_transform<t_statistic_value_type> t_transform_type>  \


namespace ropufu::aftermath::sequential
{
    template <std::totally_ordered t_observation_value_type,
        std::totally_ordered t_statistic_value_type = t_observation_value_type,
        ropufu::aftermath::sequential::timed_transform<t_statistic_value_type> t_transform_type = identity_transform<t_statistic_value_type>>
    struct window_limited_cusum;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    /** Window-limited CUSUM chart. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct window_limited_cusum
        : public window_limited_statistic<t_observation_value_type, t_statistic_value_type, t_transform_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = window_limited_statistic<t_observation_value_type, t_statistic_value_type, t_transform_type>;
        using observation_value_type = t_observation_value_type;
        using statistic_value_type = t_statistic_value_type;
        using transform_type = t_transform_type;

        using history_type = typename base_type::history_type;

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
         *  @param history Contains most recent observations (newest first, oldest last).
         */
        statistic_value_type on_history_updated(const history_type& history) noexcept override
        {
            statistic_value_type sum = 0;
            statistic_value_type max = 0;
            for (const observation_value_type& x : history)
            {
                sum += x;
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
