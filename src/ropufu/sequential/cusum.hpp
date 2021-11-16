
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../simple_vector.hpp"
#include "stopping_time.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::runtime_error
#include <string_view> // std::string_view
#include <utility>     // std::forward

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME cusum<t_value_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE \
    template <std::totally_ordered t_value_type, std::ranges::random_access_range t_container_type> \
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>           \


namespace ropufu::aftermath::sequential
{
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<t_value_type>>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct cusum;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    /** Classical CUSUM chart. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct cusum : public one_sided_stopping_time<t_value_type, t_container_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = one_sided_stopping_time<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

        using thresholds_type = typename base_type::thresholds_type;

        /** Names the stopping time type. */
        constexpr std::string_view name() const noexcept override
        {
            return "CUSUM";
        } // name(...)

        static constexpr std::size_t parameter_dim = 1;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_window_size = "window";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        // Latest statistic value.
        value_type m_statistic = 0;

    public:
        cusum() noexcept : cusum(thresholds_type{})
        {
        } // cusum(...)

        explicit cusum(const thresholds_type& thresholds)
            : base_type(thresholds)
        {
        } // cusum(...)

        explicit cusum(thresholds_type&& thresholds)
            : base_type(std::forward<thresholds_type>(thresholds))
        {
        } // cusum(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->equals(other) &&
                this->m_statistic == other.m_statistic;
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
                throw std::runtime_error("Parsing <cusum> failed: " + j.dump());
        } // from_json(...)
#endif

    protected:
        /** Processes the newest observation and returns the new value of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        value_type update_statistic(const value_type& value) noexcept override
        {
            value_type previous = this->m_statistic;
            if (previous < 0) previous = 0;
            this->m_statistic = previous + value;
            return this->m_statistic;
        } // update_statistic(...)

        /** Processes a block of newest observations and returns the new block of values of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        container_type update_statistic(const container_type& values) noexcept override
        {
            container_type result = values;
            for (std::size_t k = 0; k < values.size(); ++k)
            {
                value_type previous = this->m_statistic;
                if (previous < 0) previous = 0;
                this->m_statistic = previous + values[k];
                result[k] = this->m_statistic;
            } // for (...)

            return result;
        } // update_statistic(...)

        /** Re-initialize the chart to its original state. */
        void on_reset() noexcept override
        {
            this->m_statistic = 0;
        } // on_reset(...)

#ifndef ROPUFU_NO_JSON
        /** Serializes the class to a JSON object. */
        nlohmann::json serialize() const noexcept override
        {
            nlohmann::json j = *this;
            return j;
        } // serialize(...)
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
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!x.try_deserialize_core(j)) return false;

            std::size_t window_size = 0;
            if (!noexcept_json::optional(j, result_type::jstr_window_size, window_size)) return false;

            if (window_size != 0) return false;
            
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
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / (argument_type::parameter_dim);
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::value_type> statistic_hasher = {};

            result ^= (statistic_hasher(x.m_statistic) << ((shift * 0) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_CUSUM_HPP_INCLUDED
