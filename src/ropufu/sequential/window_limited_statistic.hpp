
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

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
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type,
        timed_transform<t_value_type> t_transform_type = identity_transform<t_value_type>>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct window_limited_statistic
        : public statistic<t_value_type, t_container_type>
    {
        using type = window_limited_statistic<t_value_type, t_container_type, t_transform_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;
        using transform_type = t_transform_type;

        /** Names the statistic type. */
        constexpr virtual std::string_view name() const noexcept = 0;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_window_size = "window";

    private:
        std::size_t m_count_observations = 0;
        // Collection of most recent observations.
        container_type m_history;
        // If L is the window size, then the structure at time n is:
        // ... --- (n - L + 1) ---  n --- (n - 1) --- (n - 2) --- ...
        //           oldest       newest
        std::size_t m_newest_index = 0;
        // Cached value of m_history.size().
        std::size_t m_window_size = 0;
        // Cached value of m_history.size() - 1.
        std::size_t m_minus_one = 0;
        // Transform for first L - 1 observations.
        transform_type m_transform = {};
        
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

        void initialize(std::size_t window_size) noexcept
        {
            this->m_history = container_type(window_size);
            this->m_window_size = this->m_history.size();
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

        void initialize(const container_type& history) noexcept
        {
            this->m_history = history;
            this->m_window_size = this->m_history.size();
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

        void initialize(container_type&& history) noexcept
        {
            this->m_history = std::move(history);
            this->m_window_size = this->m_history.size();
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

    public:
        window_limited_statistic() noexcept : window_limited_statistic(1, transform_type{})
        {
        } // window_limited_statistic(...)

        explicit window_limited_statistic(std::size_t window_size, const transform_type& transform = {})
            : m_transform(transform)
        {
            this->initialize(window_size);
            this->validate();
        } // window_limited_statistic(...)

        std::size_t window_size() const noexcept { return this->m_window_size; }

        /** The underlying process has been cleared. */
        void reset() noexcept override
        {
            for (value_type& x : this->m_history) x = 0;
            this->m_newest_index = 0;
            this->m_count_observations = 0;
            this->on_reset();
        } // on_reset(...)
        
        /** Observe a single value. */
        value_type observe(const value_type& value) noexcept override
        {
            // Element at the currently oldest index will be overwritten.
            this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
            this->m_history[this->m_newest_index] = value;

            value_type statistic = this->on_history_updated(this->m_history, this->m_newest_index);
            std::size_t time = this->m_count_observations;
            if (time < this->m_window_size) statistic = this->m_transform(time, statistic);
            ++this->m_count_observations;
            return statistic;
        } // observe(...)

        /** Observe a block of values. */
        container_type observe(const container_type& values) noexcept override
        {
            container_type statistics = values;
            std::size_t time = this->m_count_observations;

            std::size_t offset = 0;
            if (time < this->m_window_size) [[unlikely]] offset = this->m_window_size - time;

            for (std::size_t k = 0; k < offset; ++k)
            {
                this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
                this->m_history[this->m_newest_index] = values[k];

                statistics[k] = this->on_history_updated(this->m_history, this->m_newest_index);
                statistics[k] = this->m_transform(time, statistics[k]);
            } // for (...)

            for (std::size_t k = offset; k < values.size(); ++k)
            {
                this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
                this->m_history[this->m_newest_index] = values[k];

                statistics[k] = this->on_history_updated(this->m_history, this->m_newest_index);
            } // for (...)

            this->m_count_observations += values.size();
            return statistics;
        } // observe(...)
        
    protected:
        /** Occurs when the most recent observation has been added to the history.
         *  @param history If L is the window size, then the history at time n contains observations @ times:
         *  ... --- (n - L + 1) ---  n --- (n - 1) --- (n - 2) --- ...
         *            oldest       newest
         *  @param newest_index Points to the newest item in \param history.
         */
        virtual value_type on_history_updated(const container_type& history, std::size_t newest_index) noexcept = 0;

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

            std::hash<value_type> history_hasher = {};

            std::size_t offset = 0;
            for (value_type y : this->m_history)
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

            if (statistic_name != this->name()) return false;
            this->initialize(window_size);
            if (this->error_message().has_value()) return false;
            
            return true;
        } // try_deserialize_core(...)

        void serialize_core(nlohmann::json& j) const noexcept
        {
            j[std::string{type::jstr_type}] = this->name();
            j[std::string{type::jstr_window_size}] = this->m_window_size;
        } // serialize_core(...)
#endif
    }; // struct window_limited_statistic
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STATISTIC_HPP_INCLUDED
