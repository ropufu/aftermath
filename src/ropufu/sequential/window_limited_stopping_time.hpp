
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STOPPING_TIME_HPP_INCLUDED

#include "stopping_time.hpp"
#include "timed_transform.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward

namespace ropufu::aftermath::sequential
{
    /** Base class for window-limited stopping times. */
    template <stopping_time_mode t_mode,
        std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type,
        timed_transform<t_value_type> t_transform_type = identity_transform<t_value_type>>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct window_limited_stopping_time : public stopping_time<t_mode, t_value_type, t_container_type>
    {
        using type = window_limited_stopping_time<t_mode, t_value_type, t_container_type, t_transform_type>;
        using base_type = stopping_time<t_mode, t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;
        using transform_type = t_transform_type;

        using thresholds_type = typename base_type::thresholds_type;

        static constexpr stopping_time_mode mode = t_mode;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_window_size = "window";

    private:
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
        window_limited_stopping_time() noexcept : window_limited_stopping_time(1, thresholds_type{}, transform_type{})
        {
        } // window_limited_stopping_time(...)

        explicit window_limited_stopping_time(std::size_t window_size, const thresholds_type& thresholds, const transform_type& transform = {})
            : base_type(thresholds), m_transform(transform)
        {
            this->initialize(window_size);
            this->validate();
        } // window_limited_stopping_time(...)

        explicit window_limited_stopping_time(std::size_t window_size, thresholds_type&& thresholds, const transform_type& transform = {})
            : base_type(std::forward<thresholds_type>(thresholds)), m_transform(transform)
        {
            this->initialize(window_size);
            this->validate();
        } // window_limited_stopping_time(...)

        std::size_t window_size() const noexcept { return this->m_window_size; }

    protected:
        /** Processes the newest observation and returns the new value of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        value_type update_statistic(const value_type& value) noexcept override
        {
            // Element at the currently oldest index will be overwritten.
            this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
            this->m_history[this->m_newest_index] = value;

            value_type result = on_history_updated(this->m_history, this->m_newest_index);
            std::size_t time = this->count_observations();
            return (time < this->m_window_size) ?
                this->m_transform(time, result) :
                result;
        } // update_statistic(...)

        /** Processes a block of newest observations and returns the new block of values of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        container_type update_statistic(const container_type& values) noexcept override
        {
            container_type result = values;
            std::size_t time = this->count_observations();

            std::size_t offset = 0;
            if (time < this->m_window_size) [[unlikely]] offset = this->m_window_size - time;

            for (std::size_t k = 0; k < offset; ++k)
            {
                this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
                this->m_history[this->m_newest_index] = values[k];

                result[k] = on_history_updated(this->m_history, this->m_newest_index);
                result[k] = this->m_transform(time, result[k]);
            } // for (...)

            for (std::size_t k = offset; k < values.size(); ++k)
            {
                this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_window_size);
                this->m_history[this->m_newest_index] = values[k];

                result[k] = on_history_updated(this->m_history, this->m_newest_index);
            } // for (...)

            return result;
        } // update_statistic(...)

        virtual void on_reset() noexcept override
        {
            for (value_type& x : this->m_history) x = 0;
            this->m_newest_index = 0;
            this->on_reset_override();
        } // on_reset(...)
        
        /** Occurs when the most recent observation has been added to the history.
         *  @param history If L is the window size, then the history at time n contains observations @ times:
         *  ... --- (n - L + 1) ---  n --- (n - 1) --- (n - 2) --- ...
         *            oldest       newest
         *  @param newest_index Points to the newest item in \param history.
         */
        virtual value_type on_history_updated(const container_type& history, std::size_t newest_index) noexcept = 0;

        virtual void on_reset_override() noexcept = 0;

        bool equals(const type& other) const noexcept
        {
            return
                this->base_type::equals(other) &&
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
    }; // struct window_limited_stopping_time
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_STOPPING_TIME_HPP_INCLUDED
