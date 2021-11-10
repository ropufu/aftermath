
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_ONE_SIDED_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_ONE_SIDED_STOPPING_TIME_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#endif

#include "../format/cat.hpp"
#include "../ordered_vector.hpp"
#include "../vector_extender.hpp"
#include "discrete_process.hpp"
#include "observer.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <optional>    // std::optional
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward, std::move
#include <vector>      // std::vector

namespace ropufu::aftermath::sequential
{
    /** Base class for one-sided stopping times of the form inf{n : R_n > b},
     *  where R_n is the detection statistic and b is a threshold.
     */
    template <std::totally_ordered t_value_type, std::ranges::random_access_range t_container_type>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct one_sided_stopping_time : public observer<t_value_type, t_container_type>
    {
        using type = one_sided_stopping_time<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

        using thresholds_type = ropufu::ordered_vector<value_type>;
        using process_type = discrete_process<value_type, container_type>;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_thresholds = "thresholds";

    private:
        std::size_t m_count_observations = 0;
        thresholds_type m_thresholds;
        std::vector<std::size_t> m_when_stopped;
        /** If a threshold has been crossed, all smaller
         *  thresholds must have been crossed too. */
        std::size_t m_first_uncrossed_index = 0;

        void initialize(const thresholds_type& thresholds)
        {
            this->m_thresholds = thresholds;
            this->m_when_stopped = std::vector<std::size_t>(this->m_thresholds.size());
            this->m_thresholds.sort();
        } // initialize(...)

        void initialize(thresholds_type&& thresholds)
        {
            this->m_thresholds = std::move(thresholds);
            this->m_when_stopped = std::vector<std::size_t>(this->m_thresholds.size());
            this->m_thresholds.sort();
        } // initialize(...)

        void check_for_stopping(value_type statistic, std::size_t time) noexcept
        {
            while (this->m_first_uncrossed_index < this->m_thresholds.size())
            {
                // Don't do anything if the smallest threshold has not been crossed.
                if (statistic < this->m_thresholds[this->m_first_uncrossed_index]) break;

                // Smallest uncrossed threshold has been crossed. Record the stopping time...
                this->m_when_stopped[this->m_first_uncrossed_index] = time;
                // ...and move on the next thresholds.
                ++this->m_first_uncrossed_index;
            } // while (...)
        } // check_for_stopping(...)

    public:
        virtual ~one_sided_stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If the collection is empty, the rule will not run.
         */
        explicit one_sided_stopping_time(const thresholds_type& thresholds)
        {
            this->initialize(thresholds);
        } // one_sided_stopping_time(...)
        
        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If the collection is empty, the rule will not run.
         */
        explicit one_sided_stopping_time(thresholds_type&& thresholds)
        {
            this->initialize(std::forward<thresholds_type>(thresholds));
        } // one_sided_stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the rule should stop. */
        const thresholds_type& thresholds() const noexcept { return this->m_thresholds; }

        /** Number of observations when the stopping time terminated for \c this->thresholds().
         *  @remark If the process is still running 0 is returned instead.
         */
        const std::vector<std::size_t>& when() const noexcept { return this->m_when_stopped; }

        /** Number of observations when the stopping time terminated for the specific threshold. */
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
            this->on_reset();
        } // reset(...)

        /** Observe a single value. */
        void observe(value_type value) noexcept override
        {
            if (this->is_running())
            {
                value_type statistic = this->update_statistic(value);
                this->check_for_stopping(statistic, this->m_count_observations + 1);
            } // if (...)
            ++this->m_count_observations;
        } // observe(...)

        /** Observe a block of values. */
        void observe(const container_type& values) noexcept override
        {
            std::size_t n = values.size();
            if (this->is_running())
            {
                container_type statistics = this->update_statistic(values);
                
                std::size_t time = this->m_count_observations + 1;
                for (value_type statistic : statistics)
                {
                    this->check_for_stopping(statistic, time);
                    if (this->is_stopped()) break;
                    ++time;
                } // for (...)
            } // if (...)
            this->m_count_observations += n;
        } // observe(...)

        virtual std::string name() const noexcept = 0;

    protected:
        /** Processes the newest observation and returns the new value of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        virtual value_type update_statistic(const value_type& value) noexcept = 0;

        /** Processes a block of newest observations and returns the new block of values of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        virtual container_type update_statistic(const container_type& values) noexcept = 0;

        /** Re-initialize the chart to its original state. */
        virtual void on_reset() noexcept = 0;

#ifndef ROPUFU_NO_JSON
        /** Serializes the class to a JSON object. */
        virtual nlohmann::json serialize() const noexcept = 0;

        bool try_deserialize_core(const nlohmann::json& j) noexcept
        {
            thresholds_type thresholds {};
            if (!ropufu::noexcept_json::required(j, type::jstr_thresholds, thresholds)) return false;
            this->initialize(std::move(thresholds));

            return true;
        } // try_deserialize_core(...)

        void serialize_core(nlohmann::json& j) const noexcept
        {
            j[std::string{type::jstr_thresholds}] = this->m_thresholds;
        } // serialize_core(...)
#endif
    }; // struct one_sided_stopping_time
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_ONE_SIDED_STOPPING_TIME_HPP_INCLUDED
