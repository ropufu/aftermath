
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "statistic.hpp"

#include <concepts>    // std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <stdexcept>   // std::runtime_error
#include <string_view> // std::string_view
#include <utility>     // std::forward

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME cusum<t_observation_value_type, t_statistic_value_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <std::totally_ordered t_observation_value_type, std::totally_ordered t_statistic_value_type>

namespace ropufu::aftermath::sequential
{
    /** CUSUM statistic keeps trac of the maximum of all partial sums. */
    template <std::totally_ordered t_observation_value_type,
        std::totally_ordered t_statistic_value_type = t_observation_value_type>
    struct cusum;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct cusum
        : public statistic<t_observation_value_type, t_statistic_value_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using observation_value_type = t_observation_value_type;
        using statistic_value_type = t_statistic_value_type;

        /** Names the statistic. */
        static constexpr std::string_view name = "CUSUM";

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_window_size = "window";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        // Latest statistic value.
        statistic_value_type m_latest_statistic = 0;

    public:
        cusum() noexcept = default;

        /** The underlying process has been cleared. */
        void reset() noexcept override
        {
            this->m_latest_statistic = 0;
        } // reset(...)

        /** Observe a single value. */
        statistic_value_type observe(const statistic_value_type& value) noexcept override
        {
            if (this->m_latest_statistic < 0) this->m_latest_statistic = 0;
            this->m_latest_statistic += value;
            return this->m_latest_statistic;
        } // observe(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_latest_statistic == other.m_latest_statistic;
        } // operator ==(...)

        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& /*x*/) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <cusum> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct cusum
} // namespace ropufu::aftermath::sequential

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct noexcept_json_serializer<ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME>
    {
        using result_type = ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME;
        static bool try_get(const nlohmann::json& j, result_type& /*x*/) noexcept
        {
            std::string statistic_name;
            std::size_t window_size = 0;
            if (!noexcept_json::required(j, result_type::jstr_type, statistic_name)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_window_size, window_size)) return false;

            if (window_size != 0) return false;
            if (statistic_name != result_type::name) return false;
            
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

        std::size_t operator ()(const argument_type& x) const noexcept
        {
            std::size_t result = 0;
            std::hash<typename argument_type::statistic_value_type> statistic_hasher = {};

            result ^= statistic_hasher(x.m_latest_statistic);

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED
