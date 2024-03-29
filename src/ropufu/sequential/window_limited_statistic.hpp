
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

#include "../sliding_array.hpp"
#include "statistic.hpp"
#include "timed_transform.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::move

namespace ropufu::aftermath::sequential
{
    /** Implements base functionality for window-limited statistics. */
    template <std::totally_ordered t_observation_value_type,
        std::totally_ordered t_statistic_value_type,
        timed_transform<t_statistic_value_type> t_transform_type = identity_transform<t_statistic_value_type>>
    struct window_limited_statistic
        : public statistic<t_observation_value_type, t_statistic_value_type>
    {
        using type = window_limited_statistic<t_observation_value_type, t_statistic_value_type, t_transform_type>;
        using observation_value_type = t_observation_value_type;
        using statistic_value_type = t_statistic_value_type;
        using transform_type = t_transform_type;
        
        using history_type = ropufu::aftermath::sliding_vector<observation_value_type>;

        /** Names the statistic type. */
        constexpr virtual std::string_view name() const noexcept = 0;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_window_size = "window";

    private:
        std::size_t m_count_observations = 0;
        // Collection of most recent observations.
        history_type m_history;
        // Transform for first L - 1 observations.
        transform_type m_transform;
        
    protected:
        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            if (this->m_history.size() == 0) return "Window size cannot be zero.";
            return std::nullopt;
        } // error_message(...)
        
        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

    public:
        window_limited_statistic() noexcept : window_limited_statistic(1, transform_type{})
        {
        } // window_limited_statistic(...)

        explicit window_limited_statistic(std::size_t window_size, const transform_type& transform = {})
            : m_history(window_size), m_transform(transform)
        {
            this->validate();
        } // window_limited_statistic(...)

        std::size_t window_size() const noexcept { return this->m_history.size(); }

        /** The underlying process has been cleared. */
        void reset() noexcept override
        {
            this->m_history.wipe();
            this->m_count_observations = 0;
            this->on_reset();
        } // on_reset(...)
        
        /** Observe a single value. */
        statistic_value_type observe(const observation_value_type& value) noexcept override
        {
            this->m_history.displace_front(value);
            statistic_value_type statistic = this->on_history_updated(this->m_history);

            std::size_t time = this->m_count_observations;
            if (time < this->m_history.size()) statistic = this->m_transform(time, statistic);
            ++this->m_count_observations;
            return statistic;
        } // observe(...)

        /** Observe a block of values. */
        template <std::ranges::random_access_range t_observation_container_type,
            std::ranges::random_access_range t_statistic_container_type>
            requires
                std::ranges::sized_range<t_observation_container_type> &&
                std::ranges::sized_range<t_statistic_container_type> &&
                std::same_as<std::ranges::range_value_t<t_observation_container_type>, observation_value_type> &&
                std::same_as<std::ranges::range_value_t<t_statistic_container_type>, statistic_value_type>
        void observe(const t_observation_container_type& values, t_statistic_container_type& statistics) noexcept
        {
            statistics = t_statistic_container_type(values.size());
            std::size_t time = this->m_count_observations;

            std::size_t offset = 0;
            if (time < this->m_history.size()) [[unlikely]] offset = this->m_history.size() - time;

            for (std::size_t k = 0; k < offset; ++k)
            {
                this->m_history.displace_front(values[k]);
                statistics[k] = this->on_history_updated(this->m_history);
                statistics[k] = this->m_transform(time + k, statistics[k]);
            } // for (...)

            for (std::size_t k = offset; k < values.size(); ++k)
            {
                this->m_history.displace_front(values[k]);
                statistics[k] = this->on_history_updated(this->m_history);
            } // for (...)

            this->m_count_observations += values.size();
        } // observe(...)
        
    protected:
        /** Occurs when the most recent observation has been added to the history.
         *  @param history Contains most recent observations (newest first, oldest last).
         */
        virtual statistic_value_type on_history_updated(const history_type& history) noexcept = 0;

        virtual void on_reset() noexcept = 0;

        bool equals(const type& other) const noexcept
        {
            return
                this->m_history == other.m_history;
        } // equals(...)

        std::size_t get_hash() const noexcept
        {
            std::size_t result = 0;
            constexpr std::size_t total_width = sizeof(std::size_t);
            std::size_t width = total_width / (this->m_window_size);
            std::size_t shift = (width == 0 ? 1 : width);

            std::hash<observation_value_type> history_hasher = {};

            std::size_t offset = 0;
            for (const observation_value_type& y : this->m_history)
            {
                result ^= (history_hasher(y) << offset);
                offset = (offset + shift) % total_width;
            } // for (...)

            return result;
        } // operator ()(...)

#ifndef ROPUFU_NO_JSON
        bool try_deserialize_core(const nlohmann::json& j) noexcept
        {
            std::string statistic_name;
            std::size_t window_size = 0;
            if (!noexcept_json::required(j, type::jstr_type, statistic_name)) return false;
            if (!noexcept_json::required(j, type::jstr_window_size, window_size)) return false;

            this->m_history = history_type(window_size);
            if (statistic_name != this->name()) return false;
            if (this->error_message().has_value()) return false;
            
            return true;
        } // try_deserialize_core(...)

        void serialize_core(nlohmann::json& j) const noexcept
        {
            j[std::string{type::jstr_type}] = this->name();
            j[std::string{type::jstr_window_size}] = this->m_history.size();
        } // serialize_core(...)
#endif
    }; // struct window_limited_statistic
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED
