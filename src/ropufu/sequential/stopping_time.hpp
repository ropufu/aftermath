
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED

#include "../number_traits.hpp"
#include "../simple_vector.hpp"
#include "../vector_extender.hpp"
#include "statistic.hpp"

#include <algorithm>   // std::sort
#include <concepts>    // std::totally_ordered, std::constructible_from
#include <cstddef>     // std::size_t
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

namespace ropufu::aftermath::sequential
{
    namespace detail
    {
        template <typename t_value_type>
        struct stopped_module
        {
            using value_type = t_value_type;
            using statistic_type = ropufu::aftermath::simple_vector<value_type>;

        private:
            value_type m_latest = {};
            statistic_type m_statistic = {};
            
        protected:
            void on_initialized(std::size_t size) noexcept
            {
                this->m_statistic = statistic_type(size);
            } // on_initialized(...)

            void on_stopped(std::size_t i)
            {
                this->m_statistic[i] = this->m_latest;
            } // on_stopped(...)

        public:
            const statistic_type& stopped_statistic() { return this->m_statistic; }

            void if_stopped(const value_type& value) noexcept { this->m_latest = value; }
        }; // struct stopped_module

        template <>
        struct stopped_module<void>
        {
        protected:
            void on_initialized(std::size_t /*size*/) noexcept
            {
            } // on_initialized(...)

            void on_stopped(std::size_t /*i*/) noexcept
            {
            } // on_stopped(...)
        }; // struct stopped_module
    } // namespace detail

    /** Base class for one-sided stopping times of the form inf{n : R_n > b},
     *  where R_n is the detection statistic and b is a threshold.
     */
    template <std::totally_ordered t_value_type, typename t_stopped_value_type = void>
    struct stopping_time
        : public statistic<t_value_type, void>,
        public detail::stopped_module<t_stopped_value_type>
    {
        using type = stopping_time<t_value_type, t_stopped_value_type>;
        using value_type = t_value_type;
        using stopped_value_type = t_stopped_value_type;

        template <typename t_data_type>
        using vector_t = ropufu::aftermath::simple_vector<t_data_type>;
        
    private:
        std::size_t m_count_observations = 0;
        vector_t<value_type> m_thresholds = {};
        vector_t<std::size_t> m_when_stopped = {};
        /** If a threshold has been crossed, all smaller thresholds must have been crossed too. */
        std::size_t m_first_uncrossed_index = 0;

        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            for (value_type x : this->m_thresholds) if (!aftermath::is_finite(x)) return "Thresholds must be finite.";
            
            return std::nullopt;
        } // error_message(...)
        
        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

        void initialize()
        {
            this->m_when_stopped = vector_t<std::size_t>(this->m_thresholds.size());
            std::sort(this->m_thresholds.begin(), this->m_thresholds.end());
            this->on_initialized(this->m_thresholds.size());
        } // initialize(...)

    public:
        stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If the collection is empty, the rule will not run.
         */
        template <std::ranges::range t_container_type>
            requires std::constructible_from<vector_t<value_type>, t_container_type>
        explicit stopping_time(const t_container_type& thresholds)
            : m_thresholds(vector_t<value_type>(thresholds))
        {
            this->initialize();
            this->validate();
        } // stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the rule should stop. */
        const vector_t<value_type>& thresholds() const noexcept { return this->m_thresholds; }

        /** Number of observations when the stopping time terminated.
         *  @remark If the process is still running 0 is returned instead.
         */
        const vector_t<std::size_t>& when() const noexcept { return this->m_when_stopped; }

        /** Number of observations when the stopping time terminated for the indicated threshold. */
        std::size_t when(std::size_t threshold_index) const { return this->m_when_stopped[threshold_index]; }

        /** Indicates that the process has not stopped for at least one threshold. */
        bool is_running() const noexcept { return !this->is_stopped(); }

        /** Indicates that the process has stopped for all thresholds. */
        bool is_stopped() const noexcept { return this->m_first_uncrossed_index == this->m_thresholds.size(); }

        /** The underlying process has been cleared. */
        void reset() noexcept override
        {
            this->m_count_observations = 0;
            ropufu::fill(this->m_when_stopped, 0);
            this->m_first_uncrossed_index = 0;
        } // reset(...)

        /** Observe a single value. */
        void observe(const value_type& value) noexcept override
        {
            if (this->is_running())
            {
                std::size_t time = this->m_count_observations + 1;
                while (this->m_first_uncrossed_index < this->m_thresholds.size())
                {
                    // Don't do anything if the smallest threshold has not been crossed.
                    if (value <= this->m_thresholds[this->m_first_uncrossed_index]) break;

                    // Smallest uncrossed threshold has been crossed. Record the stopping time...
                    this->m_when_stopped[this->m_first_uncrossed_index] = time;
                    this->on_stopped(this->m_first_uncrossed_index);
                    // ...and move on the next thresholds.
                    ++this->m_first_uncrossed_index;
                } // while (...)
            } // if (...)
            ++this->m_count_observations;
        } // observe(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_count_observations == other.m_count_observations &&
                this->m_thresholds == other.m_thresholds;
        } // operator ==(...)

        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)
    }; // struct stopping_time
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
