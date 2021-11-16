
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../simple_vector.hpp"
#include "stopping_time.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::forward

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME window_limited_cusum<t_value_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE \
    template <std::totally_ordered t_value_type, std::ranges::random_access_range t_container_type> \
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>           \


namespace ropufu::aftermath::sequential
{
    template <std::totally_ordered t_value_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<t_value_type>>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct window_limited_cusum;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    /** Window-limited CUSUM chart. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct window_limited_cusum : public one_sided_stopping_time<t_value_type, t_container_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = one_sided_stopping_time<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

        using thresholds_type = typename base_type::thresholds_type;

        /** Names the stopping time type. */
        constexpr std::string_view name() const noexcept override
        {
            return "Window-limited CUSUM";
        } // name(...)

        // ~~ Json names ~~
        static constexpr std::string_view jstr_window_size = "window";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        // Collection of most recent observations.
        container_type m_history;
        // If L is the window size, then the structure at time n is:
        // ... --- (n - L + 1) ---  n --- (n - 1) --- (n - 2) --- ...
        //           oldest       newest
        std::size_t m_newest_index = 0;
        // Cached value of m_history.size() - 1.
        std::size_t m_minus_one = 0;
        
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
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

        void initialize(const container_type& history) noexcept
        {
            this->m_history = history;
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

        void initialize(container_type&& history) noexcept
        {
            this->m_history = std::move(history);
            this->m_minus_one = this->m_history.size() - 1;
        } // initialize(...)

    public:
        window_limited_cusum() noexcept : window_limited_cusum(1, thresholds_type{})
        {
        } // window_limited_cusum(...)

        explicit window_limited_cusum(std::size_t window_size, const thresholds_type& thresholds)
            : base_type(thresholds)
        {
            this->initialize(window_size);
            this->validate();
        } // window_limited_cusum(...)

        explicit window_limited_cusum(std::size_t window_size, thresholds_type&& thresholds)
            : base_type(std::forward<thresholds_type>(thresholds))
        {
            this->initialize(window_size);
            this->validate();
        } // window_limited_cusum(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->equals(other) &&
                this->m_history == other.m_history;
        } // operator ==(...)

        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_window_size, x.m_history.size()}
            };
            x.serialize_core(j);
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <window_limited_cusum> failed: " + j.dump());
        } // from_json(...)
#endif

    protected:
        /** Processes the newest observation and returns the new value of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        value_type update_statistic(const value_type& value) noexcept override
        {
            // Element at the currently oldest index will be overwritten.
            this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_history.size());
            this->m_history[this->m_newest_index] = value;

            value_type sum = 0;
            value_type max = 0;
            for (std::size_t i = this->m_newest_index; i < this->m_history.size(); ++i)
            {
                sum += this->m_history[i];
                if (sum > max) max = sum;
            } // for (...)
            
            for (std::size_t i = 0; i < this->m_newest_index; ++i)
            {
                sum += this->m_history[i];
                if (sum > max) max = sum;
            } // for (...)
            return max;
        } // update_statistic(...)

        /** Processes a block of newest observations and returns the new block of values of detection statistic.
         *  @remark Observation counter has not been incremented yet.
         */
        container_type update_statistic(const container_type& values) noexcept override
        {
            container_type result = values;
            for (std::size_t k = 0; k < values.size(); ++k)
            {
                this->m_newest_index = (this->m_newest_index + this->m_minus_one) % (this->m_history.size());
                this->m_history[this->m_newest_index] = values[k];

                value_type sum = 0;
                value_type max = 0;
                for (std::size_t i = this->m_newest_index; i < this->m_history.size(); ++i)
                {
                    sum += this->m_history[i];
                    if (sum > max) max = sum;
                } // for (...)

                for (std::size_t i = 0; i < this->m_newest_index; ++i)
                {
                    sum += this->m_history[i];
                    if (sum > max) max = sum;
                } // for (...)
                result[k] = max;
            } // for (...)
            
            return result;
        } // update_statistic(...)

        void on_reset() noexcept override
        {
            for (value_type& x : this->m_history) x = 0;
            this->m_newest_index = 0;
        } // on_reset(...)

#ifndef ROPUFU_NO_JSON
        /** Serializes the class to a JSON object. */
        nlohmann::json serialize() const noexcept override
        {
            nlohmann::json j = *this;
            return j;
        } // serialize(...)
#endif
    }; // struct window_limited_cusum
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

            std::size_t window_size;
            if (!noexcept_json::required(j, result_type::jstr_window_size, window_size)) return false;
            
            x.initialize(window_size);
            if (x.error_message().has_value()) return false;
            
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
            result_type width = total_width / (x.m_history.size());
            result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::value_type> statistic_hasher = {};

            result_type offset = 0;
            for (typename argument_type::value_type& y : x.m_history)
            {
                result ^= (statistic_hasher(y) << offset);
                offset = (offset + shift) % total_width;
            } // for (...)

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_WINDOW_LIMITED_CUSUM_HPP_INCLUDED
