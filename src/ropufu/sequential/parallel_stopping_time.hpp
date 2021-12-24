
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../format/cat.hpp"
#include "../algebra/matrix.hpp"
#include "../number_traits.hpp"
#include "../simple_vector.hpp"
#include "../vector_extender.hpp"
#include "statistic.hpp"

#include <algorithm>   // std::sort
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
#define ROPUFU_TMP_TYPENAME parallel_stopping_time<t_scalar_type, t_scalar_container_type, t_pair_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE                             \
    template <std::totally_ordered t_scalar_type,                 \
        std::ranges::random_access_range t_scalar_container_type, \
        std::ranges::random_access_range t_pair_container_type>   \
            requires std::same_as<std::ranges::range_value_t<t_scalar_container_type>, t_scalar_type> &&                  \
                std::same_as<std::ranges::range_value_t<t_pair_container_type>, std::pair<t_scalar_type, t_scalar_type>> \


namespace ropufu::aftermath::sequential
{
    /** Base class for two one-sided stopping times running in parallel.
     *  Equivalently, may be rewritten as inf{n : V_n > b or H_n > c},
     *  where V_n and H_n are the detection statistics and b anc c are
     *  thresholds. We will refer to V_n as the vertical statistic (frist),
     *  and H_n as the horizontal statistic (second).
     */
    template <std::totally_ordered t_scalar_type,
        std::ranges::random_access_range t_scalar_container_type = aftermath::simple_vector<t_scalar_type>,
        std::ranges::random_access_range t_pair_container_type = aftermath::simple_vector<std::pair<t_scalar_type, t_scalar_type>>>
            requires std::same_as<std::ranges::range_value_t<t_scalar_container_type>, t_scalar_type> &&
                std::same_as<std::ranges::range_value_t<t_pair_container_type>, std::pair<t_scalar_type, t_scalar_type>>
    struct parallel_stopping_time;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct parallel_stopping_time
        : public statistic<std::pair<t_scalar_type, t_scalar_type>, t_pair_container_type, void, void>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using scalar_type = t_scalar_type;
        using scalar_container_type = t_scalar_container_type;
        using pair_container_type = t_pair_container_type;

        using value_type = std::pair<t_scalar_type, t_scalar_type>;
        using thresholds_type = std::pair<scalar_container_type, scalar_container_type>;
        template <typename t_data_type>
        using matrix_t = aftermath::algebra::matrix<t_data_type>;

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
            for (scalar_type x : this->m_thresholds.first) if (!aftermath::is_finite(x)) return "Thresholds must be finite.";
            for (scalar_type x : this->m_thresholds.second) if (!aftermath::is_finite(x)) return "Thresholds must be finite.";
            
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
            this->m_which_triggered = matrix_t<char>(this->m_thresholds.first.size(), this->m_thresholds.second.size());
            this->m_when_stopped = matrix_t<std::size_t>(this->m_thresholds.first.size(), this->m_thresholds.second.size());
            std::sort(this->m_thresholds.first.begin(), this->m_thresholds.first.end());
            std::sort(this->m_thresholds.second.begin(), this->m_thresholds.second.end());
        } // initialize(...)

    public:
        parallel_stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If either collection is empty, the rule will not run.
         */
        parallel_stopping_time(const scalar_container_type& vertical_thresholds, const scalar_container_type& horizontal_thresholds)
            : m_thresholds(std::make_pair(vertical_thresholds, horizontal_thresholds))
        {
            this->initialize();
            this->validate();
        } // parallel_stopping_time(...)
        
        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If either collection is empty, the rule will not run.
         */
        parallel_stopping_time(scalar_container_type&& vertical_thresholds, scalar_container_type&& horizontal_thresholds)
            : m_thresholds(std::make_pair(
                std::forward<scalar_container_type>(vertical_thresholds),
                std::forward<scalar_container_type>(horizontal_thresholds)))
        {
            this->initialize();
            this->validate();
        } // parallel_stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the second stopping time should terminate. */
        const scalar_container_type& vertical_thresholds() const noexcept { return this->m_thresholds.first; }

        /** Thresholds, sorted in ascending order, to determine when the first stopping time should terminate. */
        const scalar_container_type& horizontal_thresholds() const noexcept { return this->m_thresholds.second; }

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
        void observe(const value_type& value) noexcept override
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
                scalar_type b = this->m_thresholds.first[i];
                if (value.first <= b) break; // The smallest uncrossed index still hasn't been crossed.
                
                next_uncrossed_vertical_index = i + 1;
                for (std::size_t j = this->m_first_uncrossed_index.second; j < n; ++j)
                {
                    this->m_which_triggered(i, j) |= 1; // Mark threshold as crossed.
                    this->m_when_stopped(i, j) = time; // Record the freshly stopped times.
                } // for (...)
            } // for (...)

            // Traverse horizontal thresholds.
            std::size_t next_uncrossed_horizontal_index = this->m_first_uncrossed_index.second;
            for (std::size_t j = this->m_first_uncrossed_index.second; j < n; ++j)
            {
                scalar_type c = this->m_thresholds.second[j];
                if (value.second <= c) break; // The smallest uncrossed index still hasn't been crossed.

                next_uncrossed_horizontal_index = j + 1;
                for (std::size_t i = this->m_first_uncrossed_index.first; i < m; ++i)
                {
                    this->m_which_triggered(i, j) |= 2; // Mark threshold as crossed.
                    this->m_when_stopped(i, j) = time; // Record the freshly stopped times.
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
            std::string stopping_time_name;
            if (!noexcept_json::required(j, result_type::jstr_type, stopping_time_name)) return false;
            if (!noexcept_json::required(j, result_type::jstr_vertical_thresholds, x.m_thresholds.first)) return false;
            if (!noexcept_json::required(j, result_type::jstr_horizontal_thresholds, x.m_thresholds.second)) return false;
            
            if (stopping_time_name != result_type::name) return false;
            if (x.error_message().has_value()) return false;
            x.initialize();

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
