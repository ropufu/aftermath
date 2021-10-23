
#ifndef ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../number_traits.hpp"
#include "concepts.hpp"

#include <cmath>       // std::sqrt
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::binomial_distribution
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string_view> // std::string_view
#include <type_traits> // std::is_floating_point_v
#include <utility>     // std::declval, std::swap

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME binomial_distribution<t_value_type, t_probability_type, t_expectation_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <ropufu::integer t_value_type, std::floating_point t_probability_type, std::floating_point t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** Binomial distribution. */
    template <ropufu::integer t_value_type = std::size_t,
        std::floating_point t_probability_type = double,
        std::floating_point t_expectation_type = decltype(std::declval<t_value_type>() * std::declval<t_probability_type>())>
    struct binomial_distribution;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct is_discrete<ROPUFU_TMP_TYPENAME>
    {
        using distribution_type = ROPUFU_TMP_TYPENAME;
        static constexpr bool value = true;
    }; // struct is_discrete

    /** @brief Binomial distribution.
     *  @todo Add tests!!
     */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct binomial_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::binomial_distribution<t_value_type>;

        static constexpr std::string_view name = "binomial";
        static constexpr std::size_t parameter_dim = 2;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_number_of_trials = "trials";
        static constexpr std::string_view jstr_probability_of_success = "success";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        value_type m_number_of_trials = 1;
        probability_type m_probability_of_success = 0;
        
        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            if constexpr (std::numeric_limits<value_type>::is_signed)
            {
                if (this->m_number_of_trials < 0) return "Number of trials cannot be negative.";
            } // if constexpr (...)
            if (this->m_number_of_trials == 0) return "Number of trials cannot be zero.";
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
        /** Trivial case with one trial that always fails. */
        binomial_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ binomial_distribution(const std_type& distribution)
            : binomial_distribution(static_cast<value_type>(distribution.t()), static_cast<probability_type>(distribution.p()))
        {
        } // binomial_distribution(...)

        /** @brief Constructs a binomial distribution from the number of trials and probability of success.
         *  @exception std::logic_error \p probability_of_success is not inside the interval [0, 1].
         */
        explicit binomial_distribution(value_type number_of_trials, probability_type probability_of_success)
            : m_number_of_trials(number_of_trials), m_probability_of_success(probability_of_success)
        {
            this->validate();
        } // binomial_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            using t_type = decltype(std::declval<std_type>().t());
            using p_type = decltype(std::declval<std_type>().p());
            return std_type(static_cast<t_type>(this->m_number_of_trials), static_cast<p_type>(this->m_probability_of_success));
        } // to_std(...)

        /** Number of trials in the experiment. */
        value_type t() const noexcept { return this->m_number_of_trials; }
        /** Number of trials in the experiment. */
        value_type number_of_trials() const noexcept { return this->m_number_of_trials; }
        /** Probability of success. */
        probability_type p() const noexcept { return this->m_probability_of_success; }
        /** Probability of success. */
        probability_type probability_of_success() const noexcept { return this->m_probability_of_success; }
        /** Probability of failure. */
        probability_type probability_of_failure() const noexcept { return (1 - this->m_probability_of_success); }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept
        {
            expectation_type n = static_cast<expectation_type>(this->m_number_of_trials);
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return n * p;
        } // expected_value(...)
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type n = static_cast<expectation_type>(this->m_number_of_trials);
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            return n * p * (1 - p);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }
        /** Smallest value in the distribution. */
        constexpr value_type min() const noexcept { return 0; }
        /** Largest value in the distribution. */
        value_type max() const noexcept { return this->m_number_of_trials; }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            if (k >= this->m_number_of_trials) return 1;
            
            probability_type p = 0;
            for (value_type j = 0; j <= k; ++j) p += this->pmf(j);
            return p;
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            expectation_type p = static_cast<expectation_type>(this->m_probability_of_success);
            expectation_type q = 1 - p;
            value_type n = this->m_number_of_trials;

            if (k > n || k < 0) return 0;
            if (k > n / 2)
            {
                k = n - k;
                std::swap(p, q);
            } // if (...)
            if (q == 0) return 0;
            value_type m = n - 2 * k;

            expectation_type result = static_cast<expectation_type>(scale);
            expectation_type v = p * q;
            expectation_type numerator = static_cast<expectation_type>(n - k);
            for (expectation_type i = 1; i <= k; ++i) result *= (++numerator) * v / i;
            for (value_type j = 0; j < m; ++j) result *= q;
            return static_cast<probability_type>(result);
        } // pmf(...)

        /** Support of the distribution. */
        std::vector<value_type> support() const
        {
            std::vector<value_type> result {};
            result.reserve(static_cast<std::size_t>(this->m_number_of_trials) + 1);
            for (value_type k = 0; k <= this->m_number_of_trials; ++k) result.push_back(k);
            return result;
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_number_of_trials == other.m_number_of_trials &&
                this->m_probability_of_success == other.m_probability_of_success;
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

            if (x.m_number_of_trials != default_instance.m_number_of_trials) j[std::string(type::jstr_number_of_trials)] = x.m_number_of_trials;
            if (x.m_probability_of_success != default_instance.m_probability_of_success) j[std::string(type::jstr_probability_of_success)] = x.m_probability_of_success;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <binomial_distribution> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct binomial_distribution
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
            if (!noexcept_json::optional(j, result_type::jstr_number_of_trials, x.m_number_of_trials)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_probability_of_success, x.m_probability_of_success)) return false;

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

            std::hash<typename argument_type::mu_type> number_of_trials_hasher = {};
            std::hash<typename argument_type::mu_type> probability_of_success_hasher = {};

            result ^= (number_of_trials_hasher(x.m_number_of_trials) << ((shift * 0) % total_width));
            result ^= (probability_of_success_hasher(x.m_probability_of_success) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_BINOMIAL_DISTRIBUTION_HPP_INCLUDED
