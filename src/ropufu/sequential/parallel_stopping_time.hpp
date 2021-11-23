
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../algebra/matrix.hpp"
#include "../number_traits.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward, std::move, std::pair
#include <vector>      // std::vector

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME parallel_stopping_time<t_value_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE \
    template <std::totally_ordered t_value_type, std::ranges::random_access_range t_container_type> \
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>           \


namespace ropufu::aftermath::sequential
{
    /** Base class for two one-sided stopping times running in parallel.
     *  Equivalently, may be rewritten as inf{n : V_n > b or H_n > c},
     *  where V_n and H_n are the detection statistics and b anc c are
     *  thresholds. We will refer to V_n as the vertical statistic (frist),
     *  and H_n as the horizontal statistic (second).
     */
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<t_value_type>>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct parallel_stopping_time;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct parallel_stopping_time
        : public statistic<t_value_type, t_container_type, void, void>
    {
        using type = stopping_time<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

        using thresholds_type = ropufu::ordered_vector<value_type>;
        template <typename t_data_type>
        using matrix_t = aftermath::algebra::matrix<t_data_type>;
        template <typename t_type>
        using pair_t = std::pair<t_type, t_type>;

        /** Names the stopping time. */
        static constexpr std::string_view name = "parallel";

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_vertical_thresholds = "vertical thresholds";
        static constexpr std::string_view jstr_horizontal_thresholds = "horizontal thresholds";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        std::size_t m_count_observations = 0;
        /** Two vectors of thresholds for the first and second statistic. */
        pair_t<thresholds_type> m_thresholds = {};
        /** Matrix indicating which statistic caused stopping (1 for first, 2 for second, 3 for both). */
        matrix_t<char> m_which_triggered = {};
        /** Matrix counting the number of observations prior to stopping. */
        matrix_t<std::size_t> m_when_stopped = {};

        /** Keep track of the first uncrossed threshold index for each of the statistics. */
        pair_t<std::size_t> m_first_uncrossed_index = 0;

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

        void initialize(const pair_t<thresholds_type>& thresholds)
        {
            this->m_thresholds = thresholds;
            this->on_thresholds_initialized();
        } // initialize(...)

        void initialize(pair_t<thresholds_type>&& thresholds)
        {
            this->m_thresholds = std::move(thresholds);
            this->on_thresholds_initialized();
        } // initialize(...)

        void on_thresholds_initialized()
        {
            this->m_which_triggered = matrix_t<std::byte>(this->m_vertical_thresholds.size(), this->m_horizontal_thresholds.size());
            this->m_when_stopped = matrix_t<std::size_t>(this->m_vertical_thresholds.size(), this->m_horizontal_thresholds.size());
            this->m_thresholds.first.sort();
            this->m_thresholds.second.sort();
        } // on_thresholds_initialized(...)

    public:
        parallel_stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If either collection is empty, the rule will not run.
         */
        parallel_stopping_time(const thresholds_type& vertical_thresholds, const thresholds_type& horizontal_thresholds)
        {
            this->initialize(std::make_pair(vertical_thresholds, horizontal_thresholds));
            this->validate();
        } // parallel_stopping_time(...)
        
        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If either collection is empty, the rule will not run.
         */
        parallel_stopping_time(thresholds_type&& vertical_thresholds, thresholds_type&& horizontal_thresholds)
        {
            this->initialize(std::make_pair(
                std::forward<thresholds_type>(vertical_thresholds),
                std::forward<thresholds_type>(horizontal_thresholds)
            ));
            this->validate();
        } // parallel_stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the second stopping time should terminate. */
        const thresholds_type& vertical_thresholds() const noexcept { return this->m_thresholds.first; }

        /** Thresholds, sorted in ascending order, to determine when the first stopping time should terminate. */
        const thresholds_type& horizontal_thresholds() const noexcept { return this->m_thresholds.second; }

        /** Which of the rules caused termination: 1 for first, 2 for second, 3 for both, 0 for neither. */
        const matrix_t<std::size_t>& which() const noexcept { return this->m_when_stopped; }

        /** Which of the rules caused termination for the indicated threshold. */
        std::size_t which(std::size_t vertical_threshold_index, std::size_t horizontal_threshold_index) const
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
                this->m_first_uncrossed_index.first == this->m_thresholds.first.size() &&
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
        void observe(value_type value) noexcept override
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

            // Traverse vertical thresholds.
            std::size_t next_uncrossed_vertical_index = this->m_first_uncrossed_index.first;
            for (std::size_t i = this->m_first_uncrossed_index.first; i < m; ++i)
            {
                value_type b = this->m_thresholds.first[i];
                if (!this->do_decide_null(b, i, 0)) break; // Break the loop the first time the null hypothesis is not accepted.
                else
                {
                    next_uncrossed_vertical_index = i + 1;
                    for (std::size_t j = this->m_first_uncrossed_index.second; j < n; ++j)
                    {
                        this->m_which_triggered(i, j) |= 1; // Mark threshold as crossed.
                        this->m_run_length(i, j) = proc.count(); // Record the freshly stopped times.
                    } // for (...)
                } // else (...)
            } // for (...)

            // Traverse horizontal thresholds.
            std::size_t next_uncrossed_alt_index = this->m_first_uncrossed_alt_index;
            for (std::size_t j = this->m_first_uncrossed_alt_index; j < n; ++j)
            {
                value_type b = this->m_unscaled_alt_thresholds[j];
                if (!this->do_decide_alt(b, 0, j)) break; // Break the loop the first time the alternative hypothesis is not accepted.
                else
                {
                    next_uncrossed_alt_index = j + 1;
                    for (std::size_t i = this->m_first_uncrossed_null_index; i < m; ++i)
                    {
                        this->m_has_decided_alt(i, j) = true; // Mark threshold as crossed.
                        this->m_run_length(i, j) = proc.count(); // Record the freshly stopped times.
                    } // for (...)
                } // else (...)
            } // for (...)

            this->m_first_uncrossed_null_index = next_uncrossed_null_index;
            this->m_first_uncrossed_alt_index = next_uncrossed_alt_index;
            
            bool is_still_listening = (this->m_first_uncrossed_null_index < m) && (this->m_first_uncrossed_alt_index < n);
            if (!is_still_listening) this->m_state = two_sprt_state::decided;

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

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name},
                {type::jstr_vertical_thresholds, x.m_thresholds.first},
                {type::jstr_horizontal_thresholds, x.m_thresholds.second}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <parallel_stopping_time> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct parallel_stopping_time
} // namespace ropufu::sequential::hypotheses

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
