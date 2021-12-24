
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../format/cat.hpp"
#include "../number_traits.hpp"
#include "../simple_vector.hpp"
#include "../vector_extender.hpp"
#include "statistic.hpp"

#include <algorithm>   // std::sort
#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <ranges>      // std::ranges::...
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward, std::move
#include <vector>      // std::vector

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME stopping_time<t_value_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE \
    template <std::totally_ordered t_value_type, std::ranges::random_access_range t_container_type> \
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>           \


namespace ropufu::aftermath::sequential
{
    /** Base class for one-sided stopping times of the form inf{n : R_n > b},
     *  where R_n is the detection statistic and b is a threshold.
     */
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<t_value_type>>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct stopping_time;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct stopping_time
        : public statistic<t_value_type, t_container_type, void, void>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = t_value_type;
        using container_type = t_container_type;

        /** Names the stopping time. */
        static constexpr std::string_view name = "one-sided";

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_thresholds = "thresholds";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;
        
    private:
        std::size_t m_count_observations = 0;
        container_type m_thresholds = {};
        std::vector<std::size_t> m_when_stopped = {};
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
            this->m_when_stopped = std::vector<std::size_t>(this->m_thresholds.size());
            std::sort(this->m_thresholds.begin(), this->m_thresholds.end());
        } // initialize(...)

        void check_for_stopping(value_type statistic, std::size_t time) noexcept
        {
            while (this->m_first_uncrossed_index < this->m_thresholds.size())
            {
                // Don't do anything if the smallest threshold has not been crossed.
                if (statistic <= this->m_thresholds[this->m_first_uncrossed_index]) break;

                // Smallest uncrossed threshold has been crossed. Record the stopping time...
                this->m_when_stopped[this->m_first_uncrossed_index] = time;
                // ...and move on the next thresholds.
                ++this->m_first_uncrossed_index;
            } // while (...)
        } // check_for_stopping(...)

    public:
        stopping_time() noexcept = default;

        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If the collection is empty, the rule will not run.
         */
        explicit stopping_time(const container_type& thresholds)
            : m_thresholds(thresholds)
        {
            this->initialize();
            this->validate();
        } // stopping_time(...)
        
        /** Initializeds the stopping time for a given collection of thresholds.
         *  @remark If the collection is empty, the rule will not run.
         */
        explicit stopping_time(container_type&& thresholds)
            : m_thresholds(std::forward<container_type>(thresholds))
        {
            this->initialize();
            this->validate();
        } // stopping_time(...)

        std::size_t count_observations() const noexcept { return this->m_count_observations; }

        /** Thresholds, sorted in ascending order, to determine when the rule should stop. */
        const container_type& thresholds() const noexcept { return this->m_thresholds; }

        /** Number of observations when the stopping time terminated.
         *  @remark If the process is still running 0 is returned instead.
         */
        const std::vector<std::size_t>& when() const noexcept { return this->m_when_stopped; }

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
                this->check_for_stopping(value, this->m_count_observations + 1);
            } // if (...)
            ++this->m_count_observations;
        } // observe(...)

        /** Observe a block of values. */
        void observe(const container_type& values) noexcept override
        {
            std::size_t n = values.size();
            if (this->is_running())
            {
                std::size_t time = this->m_count_observations + 1;
                for (value_type x : values)
                {
                    this->check_for_stopping(x, time);
                    if (this->is_stopped()) break;
                    ++time;
                } // for (...)
            } // if (...)
            this->m_count_observations += n;
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
                {type::jstr_thresholds, x.m_thresholds}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <stopping_time> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct stopping_time
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
            if (!noexcept_json::required(j, result_type::jstr_thresholds, x.m_thresholds)) return false;
            
            if (stopping_time_name != result_type::name) return false;
            if (x.error_message().has_value()) return false;
            x.initialize();

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
