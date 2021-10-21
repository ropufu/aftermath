
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"

#include "../number_traits.hpp"
#include "concepts.hpp"

#include <cmath>       // std::sqrt
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::bernoulli_distribution
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string_view> // std::string_view
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval
#include <vector>      // std::vector

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME bernoulli_distribution<t_probability_type, t_expectation_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <std::floating_point t_probability_type, std::floating_point t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** Bernoulli distribution. */
    template <std::floating_point t_probability_type = double,
        std::floating_point t_expectation_type = t_probability_type>
    struct bernoulli_distribution;

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct is_discrete<ROPUFU_TMP_TYPENAME>
    {
        using distribution_type = ROPUFU_TMP_TYPENAME;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Bernoulli distribution.
     *  @todo Add tests!!
     */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct bernoulli_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = bool;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::bernoulli_distribution;

        static constexpr std::string_view name = "bernoulli";
        static constexpr std::size_t parameter_dim = 1;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_probability_of_success = "success";
        
        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    private:
        probability_type m_probability_of_success = 0;
        
        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            if (!aftermath::is_probability(this->m_probability_of_success)) return "Probability of success must be between 0 and 1.";
            
            return std::nullopt;
        } // error_message(...)
        
        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

    public:
        /** Trivial case when trials always fail. */
        bernoulli_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ bernoulli_distribution(const std_type& distribution)
            : bernoulli_distribution(static_cast<probability_type>(distribution.p()))
        {
        } // bernoulli_distribution(...)

        /** @exception std::logic_error \p probability_of_success is not inside the interval [0, 1]. */
        explicit bernoulli_distribution(probability_type probability_of_success)
            : m_probability_of_success(probability_of_success)
        {
            this->validate();
        } // bernoulli_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using p_type = decltype(std::declval<std_type>().p());
            return std_type(static_cast<p_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Probability of success. */
        probability_type p() const noexcept { return this->m_probability_of_success; }
        /** Probability of success. */
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }
        /** Probability of failure. */
        probability_type probability_of_failure() const noexcept { return (1 - this->m_probability_of_success); }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return static_cast<expectation_type>(this->m_probability_of_success); }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return p * (1 - p);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            return k ? 1 : (1 - this->m_probability_of_success);
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            return k ?
                (scale * this->m_probability_of_success) :
                (scale - scale * this->m_probability_of_success);
        } // pmf(...)

        /** Support of the distribution. */
        std::vector<value_type> support() const noexcept
        {
            return {false, true};
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_probability_of_success == other.m_probability_of_success;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const
        {
            return !this->operator ==(other);
        } // operator !=(...)

        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };

            static type default_instance {};

            if (x.m_probability_of_success != default_instance.m_probability_of_success) j[std::string(type::jstr_probability_of_success)] = x.m_probability_of_success;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <bernoulli_distribution> failed: " + j.dump());
        } // from_json(...)
    }; // struct bernoulli_distribution
} // namespace ropufu::aftermath::probability

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
            if (!noexcept_json::optional(j, result_type::jstr_probability_of_success, x.m_probability_of_success)) return false;

            if (distribution_name != result_type::name) return false;

            if (x.error_message().has_value()) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct hash<ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME>
    {
        using argument_type = ropufu::aftermath::probability::ROPUFU_TMP_TYPENAME;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            result_type result = 0;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / (argument_type::parameter_dim);
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::mu_type> probability_of_success_hasher = {};

            result ^= (probability_of_success_hasher(x.m_probability_of_success) << ((shift * 0) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BERNOULLI_DISTRIBUTION_HPP_INCLUDED
