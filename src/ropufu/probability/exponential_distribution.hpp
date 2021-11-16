
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../number_traits.hpp"
#include "concepts.hpp"
#include "standard_exponential_distribution.hpp"

#include <cmath>       // std::sqrt, std::pow, std::erfc
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::exponential_distribution
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME exponential_distribution<t_value_type, t_probability_type, t_expectation_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <std::floating_point t_value_type, std::floating_point t_probability_type, std::floating_point t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** @brief Exponential distribution. */
    template <std::floating_point t_value_type = double,
        std::floating_point t_probability_type = t_value_type,
        std::floating_point t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct exponential_distribution;

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

    /** @brief Exponential distribution. */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct exponential_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using fundamental_type = standard_exponential_distribution<t_value_type, t_probability_type, t_expectation_type>;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::exponential_distribution<t_value_type>;

        static constexpr std::string_view name = "exponential";
        static constexpr std::size_t parameter_dim = 1;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_lambda = "rate";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        fundamental_type m_fundamental_distribution = {};
        value_type m_lambda = 1; // Rate.
        // ~~ Cached values ~~
        expectation_type m_cache_mean = 1;
        expectation_type m_cache_variance = 1;
        expectation_type m_cache_pdf_scale = 1;
        
        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            if (!aftermath::is_finite(this->m_lambda)) return "Rate must be finite.";
            if (this->m_lambda <= 0) return "Rate must be positive.";
            
            return std::nullopt;
        } // error_message(...)
        
        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

        void cahce() noexcept
        {
            this->m_cache_mean = 1 / static_cast<expectation_type>(this->m_lambda);
            this->m_cache_variance = this->m_cache_mean * this->m_cache_mean;
            this->m_cache_pdf_scale = static_cast<expectation_type>(this->m_lambda);
        } // coerce(...)

    public:
        /** Default constructor with with unit mean / rate. */
        exponential_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ exponential_distribution(const std_type& distribution)
            : exponential_distribution(static_cast<value_type>(distribution.lambda()))
        {
        } // exponential_distribution(...)

        /** @brief Constructs a exponential distribution from the mean and standard deviation.
         *  @exception std::logic_error \p lambda is not positive.
         */
        exponential_distribution(value_type lambda)
            : m_lambda(lambda)
        {
            this->validate();
            this->cahce();
        } // exponential_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using lambda_type = decltype(std::declval<std_type>().lambda());
            return std_type(static_cast<lambda_type>(this->m_lambda));
        } // to_std(...)

        /** Rate of the distribution. */
        value_type lambda() const noexcept { return this->m_lambda; }
        /** Rate of the distribution. */
        value_type rate() const noexcept { return this->m_lambda; }
        /** Mean of the distribution. */
        value_type mu() const noexcept { return 1 / this->m_lambda; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept { return this->m_cache_mean; }
        /** Variance of the distribution. */
        expectation_type variance() const noexcept { return this->m_cache_variance; }
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return this->m_cache_mean; }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }

        /** Cumulative distribution function (c.d.f.) of the distribution. */   
        probability_type cdf(value_type x) const noexcept
        {
            return this->m_fundamental_distribution.cdf(this->m_lambda * x);
        } // cdf(...)

        /** Probability density function (p.d.f.) of the distribution. */
        expectation_type pdf(value_type x) const noexcept
        {
            return this->m_cache_pdf_scale * this->m_fundamental_distribution.pdf(this->m_lambda * x);
        } // pdf(...)
        
        /** @brief For a given 0 <= p <= 1, find t such that cdf(t) = p.
         *  @exception std::logic_error \p p is not inside the interval [0, 1].
         */
        expectation_type quantile(probability_type p) const
        {
            return this->m_cache_mean * this->m_fundamental_distribution.quantile(p);
        } // quantile(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_lambda == other.m_lambda;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };

            static type default_instance {};

            if (x.m_lambda != default_instance.m_lambda) j[std::string(type::jstr_lambda)] = x.m_lambda;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <exponential_distribution> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct exponential_distribution
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
            if (!noexcept_json::optional(j, result_type::jstr_lambda, x.m_lambda)) return false;

            if (distribution_name != result_type::name) return false;

            if (x.error_message().has_value()) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

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

            std::hash<typename argument_type::mu_type> lambda_hasher = {};

            result ^= (lambda_hasher(x.m_lambda) << ((shift * 0) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_EXPONENTIAL_DISTRIBUTION_HPP_INCLUDED
