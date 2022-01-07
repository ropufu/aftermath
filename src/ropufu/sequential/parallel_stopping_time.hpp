
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED

#include "../algebra/matrix.hpp"
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
#include <utility>     // std::pair

namespace ropufu::aftermath::sequential
{
    namespace detail
    {
        template <typename t_value_type>
        struct parallel_stopped_module
        {
            using value_type = t_value_type;
            using statistic_type = ropufu::aftermath::algebra::matrix<value_type>;

        private:
            value_type m_latest = {};
            statistic_type m_statistic = {};
            
        protected:
            void on_initialized(std::size_t height, std::size_t width) noexcept
            {
                this->m_statistic = statistic_type(height, width);
            } // on_initialized(...)

            void on_stopped(std::size_t i, std::size_t j)
            {
                this->m_statistic(i, j) = this->m_latest;
            } // on_stopped(...)

        public:
            const statistic_type& stopped_statistic() const noexcept { return this->m_statistic; }

            void if_stopped(const value_type& value) noexcept { this->m_latest = value; }
        }; // struct parallel_stopped_module

        template <>
        struct parallel_stopped_module<void>
        {
        protected:
            void on_initialized(std::size_t /*height*/, std::size_t /*width*/) noexcept
            {
            } // on_initialized(...)

            void on_stopped(std::size_t /*i*/, std::size_t /*j*/) noexcept
            {
            } // on_stopped(...)
        }; // struct parallel_stopped_module
    } // namespace detail

    /** Base class for two one-sided stopping times running in parallel.
     *  Equivalently, may be rewritten as inf{n : V_n > b or H_n > c},
     *  where V_n and H_n are the detection statistics and b anc c are
     *  thresholds. We will refer to V_n as the vertical statistic (frist),
     *  and H_n as the horizontal statistic (second).
     */
    template <std::totally_ordered t_value_type, typename t_stopped_value_type = void>
    struct parallel_stopping_time
        : public statistic<std::pair<t_value_type, t_value_type>, void>,
        public detail::parallel_stopped_module<t_stopped_value_type>
    {
        using type = parallel_stopping_time<t_value_type, t_stopped_value_type>;
        using value_type = t_value_type;
        using stopped_value_type = t_stopped_value_type;

        template <typename t_data_type>
        using matrix_t = ropufu::aftermath::algebra::matrix<t_data_type>;
        using thresholds_container_type = ropufu::aftermath::simple_vector<value_type>;
        using thresholds_type = std::pair<thresholds_container_type, thresholds_container_type>;

        static constexpr char decide_vertical   = 0b001;
        static constexpr char decide_horizontal = 0b010;
        static constexpr char decide_error      = type::decide_vertical | type::decide_horizontal;

    private:
        std::size_t m_count_observations = 0;
        /** Two vectors of thresholds for the first and second statistic. */
        thresholds_type m_thresholds = {};
        /** Matrix indicating which statistic caused stopping (1 for first, 2 for second, 3 for both). */
        matrix_t<char> m_which_triggered = {};
        /** Matrix counting the number of observations prior to stopping. */
        matrix_t<std::size_t> m_when_stopped = {};
        /** Keep track of the first uncrossed threshold index for each of the statistics. */
        std::pair<std::size_t, std::size_t> m_first_uncrossed_index = {};

        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            for (value_type x : this->m_thresholds.first) if (!aftermath::is_finite(x)) return "Thresholds must be finite.";
            for (value_type x : this->m_thresholds.second) if (!aftermath::is_finite(x)) return "Thresholds must be finite.";
            
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
            std::size_t height = this->m_thresholds.first.size();
            std::size_t width = this->m_thresholds.second.size();
            this->m_which_triggered = matrix_t<char>(height, width);
            this->m_when_stopped = matrix_t<std::size_t>(height, width);
            std::sort(this->m_thresholds.first.begin(), this->m_thresholds.first.end());
            std::sort(this->m_thresholds.second.begin(), this->m_thresholds.second.end());
            this->on_initialized(height, width);
        } // initialize(...)

    public:
        parallel_stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If either collection is empty, the rule will not run.
         */
        template <std::ranges::range t_container_type>
            requires std::constructible_from<thresholds_container_type, t_container_type>
        parallel_stopping_time(const t_container_type& vertical_thresholds, const t_container_type& horizontal_thresholds)
            : m_thresholds(std::make_pair(
                thresholds_container_type(vertical_thresholds),
                thresholds_container_type(horizontal_thresholds)))
        {
            this->initialize();
            this->validate();
        } // parallel_stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the second stopping time should terminate. */
        const thresholds_container_type& vertical_thresholds() const noexcept { return this->m_thresholds.first; }

        /** Thresholds, sorted in ascending order, to determine when the first stopping time should terminate. */
        const thresholds_container_type& horizontal_thresholds() const noexcept { return this->m_thresholds.second; }

        /** Which of the rules caused termination: 1 for first, 2 for second, 3 for both, 0 for neither. */
        const matrix_t<char>& which() const noexcept { return this->m_which_triggered; }

        /** Which of the rules caused termination for the indicated threshold. */
        char which(std::size_t vertical_threshold_index, std::size_t horizontal_threshold_index) const
        {
            return this->m_which_triggered(vertical_threshold_index, horizontal_threshold_index);
        } // which(...)

        /** Number of observations when the stopping time terminated.
         *  @remark If the process is still running 0 is returned instead.
         */
        const matrix_t<std::size_t>& when() const noexcept { return this->m_when_stopped; }

        /** Number of observations when the stopping time terminated for the indicated threshold. */
        std::size_t when(std::size_t vertical_threshold_index, std::size_t horizontal_threshold_index) const
        {
            return this->m_when_stopped(vertical_threshold_index, horizontal_threshold_index);
        } // when(...)

        /** Indicates that the process has not stopped for at least one threshold. */
        bool is_running() const noexcept { return !this->is_stopped(); }

        /** Indicates that the process has stopped for all thresholds. */
        bool is_stopped() const noexcept
        {
            return
                this->m_first_uncrossed_index.first == this->m_thresholds.first.size() ||
                this->m_first_uncrossed_index.second == this->m_thresholds.second.size();
        } // is_stopped(...)

        /** The underlying process has been cleared. */
        void reset() noexcept override
        {
            this->m_count_observations = 0;
            this->m_which_triggered.wipe();
            this->m_when_stopped.wipe();
            this->m_first_uncrossed_index.first = 0;
            this->m_first_uncrossed_index.second = 0;
        } // reset(...)

        /** Observe a single value. */
        void observe(const std::pair<value_type, value_type>& value) noexcept override
        {
            //         |  0    1   ...   n-1    | c (horizontal) 
            // --------|------------------------|         
            //     0   |           ...          |         
            //     1   |           ...          |         
            //    ...  |           ...          |         
            //    m-1  |           ...          |         
            // ----------------------------------         
            //  b (vertical)                                  
            //
            
            std::size_t m = this->m_thresholds.first.size(); // Height of the threshold matrix.
            std::size_t n = this->m_thresholds.second.size(); // Width of the threshold matrix.
            std::size_t time = this->m_count_observations + 1;

            // Traverse vertical thresholds.
            std::size_t next_uncrossed_vertical_index = this->m_first_uncrossed_index.first;
            for (std::size_t i = this->m_first_uncrossed_index.first; i < m; ++i)
            {
                value_type b = this->m_thresholds.first[i];
                if (value.first <= b) break; // The smallest uncrossed index still hasn't been crossed.
                
                next_uncrossed_vertical_index = i + 1;
                for (std::size_t j = this->m_first_uncrossed_index.second; j < n; ++j)
                {
                    this->m_which_triggered(i, j) |= type::decide_vertical; // Mark threshold as crossed.
                    this->m_when_stopped(i, j) = time; // Record the freshly stopped times.
                    this->on_stopped(i, j);
                } // for (...)
            } // for (...)

            // Traverse horizontal thresholds.
            std::size_t next_uncrossed_horizontal_index = this->m_first_uncrossed_index.second;
            for (std::size_t j = this->m_first_uncrossed_index.second; j < n; ++j)
            {
                value_type c = this->m_thresholds.second[j];
                if (value.second <= c) break; // The smallest uncrossed index still hasn't been crossed.

                next_uncrossed_horizontal_index = j + 1;
                for (std::size_t i = this->m_first_uncrossed_index.first; i < m; ++i)
                {
                    this->m_which_triggered(i, j) |= type::decide_horizontal; // Mark threshold as crossed.
                    this->m_when_stopped(i, j) = time; // Record the freshly stopped times.
                    this->on_stopped(i, j);
                } // for (...)
            } // for (...)

            this->m_first_uncrossed_index.first = next_uncrossed_vertical_index;
            this->m_first_uncrossed_index.second = next_uncrossed_horizontal_index;

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
    }; // struct parallel_stopping_time
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
