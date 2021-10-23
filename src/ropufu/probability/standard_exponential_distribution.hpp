
#ifndef ROPUFU_AFTERMATH_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../number_traits.hpp"
#include "concepts.hpp"

#include <cmath>       // std::exp, std::log
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::exponential_distribution
#include <stdexcept>   // std::logic_error
#include <string_view> // std::string_view
#include <utility>     // std::declval

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME standard_exponential_distribution<t_value_type, t_probability_type, t_expectation_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <std::floating_point t_value_type, std::floating_point t_probability_type, std::floating_point t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** @brief Exponential distribution with unit mean / rate. */
    template <std::floating_point t_value_type = double,
        std::floating_point t_probability_type = t_value_type,
        std::floating_point t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct standard_exponential_distribution;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct is_continuous<ROPUFU_TMP_TYPENAME>
    {
        using distribution_type = ROPUFU_TMP_TYPENAME;
        static constexpr bool value = true;
    }; // struct is_continuous

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct has_right_tail<ROPUFU_TMP_TYPENAME>
    {
        using distribution_type = ROPUFU_TMP_TYPENAME;
        static constexpr bool value = true;
    }; // struct has_right_tail

    /** @brief Exponential distribution with unit mean / rate. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct standard_exponential_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::exponential_distribution<t_value_type>;

        static constexpr std::string_view name = "std exponential";
        static constexpr std::size_t parameter_dim = 0;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        /** @brief Validates the structure and returns an error message, if any. */
        constexpr std::optional<std::string> error_message() const noexcept
        {
            return std::nullopt;
        } // error_message(...)
        
        constexpr void validate() const noexcept { }

    public:
        /** Default constructor with zero mean and unit variance. */
        constexpr standard_exponential_distribution() noexcept { }

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(1);
        } // to_std(...)

        /** Rate of the distribution. */
        constexpr value_type lambda() const noexcept { return 1; }
        /** Rate of the distribution. */
        constexpr value_type rate() const noexcept { return 1; }
        /** Mean of the distribution. */
        constexpr value_type mu() const noexcept { return 1; }

        /** Expected value of the distribution. */
        constexpr expectation_type expected_value() const noexcept { return 1; }
        /** Variance of the distribution. */
        constexpr expectation_type variance() const noexcept { return 1; }
        /** Standard deviation of the distribution. */
        constexpr expectation_type standard_deviation() const noexcept { return 1; }

        /** Expected value of the distribution. */
        constexpr expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        constexpr expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type x) const noexcept
        {
            if constexpr (std::numeric_limits<value_type>::is_signed)
            {
                if (x < 0) return 0;
            } // if constexpr (...)
            return 1 - std::exp(-static_cast<probability_type>(x));
        } // cdf(...)

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const
        {
            if constexpr (std::numeric_limits<value_type>::is_signed)
            {
                if (x < 0) return 0;
            } // if constexpr (...)
            return std::exp(-static_cast<expectation_type>(x));
        } // pdf(...)
        
        /** @brief For a given 0 <= p <= 1, find t such that cdf(t) = p.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         */
        expectation_type quantile(probability_type p) const
        {
            if (!aftermath::is_probability(p)) throw std::logic_error("Probability must be a finite number between 0 and 1.");
            return -std::log(1 - static_cast<expectation_type>(p));
        } // quantile(...)

        /** Checks if the two distributions are the same. */
        constexpr bool operator ==(const type& other) const noexcept { return true; }

        /** Checks if the two distributions are different. */
        constexpr bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x) noexcept { }
#endif
    }; // struct standard_exponential_distribution
} // namespace ropufu::aftermath::probability

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct noexcept_json_serializer<ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME>
    {
        using result_type = ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            std::string distribution_name;

            if (!noexcept_json::required(j, result_type::jstr_type, distribution_name)) return false;

            if (distribution_name != result_type::name) return false;

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

namespace std
{
    template <typename t_value_type, typename t_probability_type, typename t_expectation_type>
    struct hash<ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME>
    {
        using argument_type = ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            return 0;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_STANDARD_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
