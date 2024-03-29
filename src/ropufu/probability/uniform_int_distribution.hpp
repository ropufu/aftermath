
#ifndef ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../number_traits.hpp"
#include "../math_constants.hpp"
#include "concepts.hpp"

#include <cmath>       // std::sqrt
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <limits>      // std::numeric_limits
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::uniform_int_distribution
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
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
#define ROPUFU_TMP_TYPENAME uniform_int_distribution<t_value_type, t_probability_type, t_expectation_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <ropufu::integer t_value_type, std::floating_point t_probability_type, std::floating_point t_expectation_type>

namespace ropufu::aftermath::probability
{
    /** Discrete uniform distribution. */
    template <ropufu::integer t_value_type = std::size_t,
        std::floating_point t_probability_type = double,
        std::floating_point t_expectation_type = t_probability_type>
    struct uniform_int_distribution;

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

    /** @brief Discrete uniform distribution.
     *  @todo Add tests!!
     */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct uniform_int_distribution : distribution_base<ROPUFU_TMP_TYPENAME>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using std_type = std::uniform_int_distribution<value_type>;

        static constexpr std::string_view name = "uniform int";
        static constexpr std::size_t parameter_dim = 2;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_min = "min";
        static constexpr std::string_view jstr_max = "max";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        value_type m_min = 0;
        value_type m_max = 1;
        
        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            if (this->m_min > this->m_max) return "Smallest value cannot exceed than the largest value.";
            
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
        uniform_int_distribution() noexcept { }

        /** Constructor and implicit conversion from standard distribution. */
        /*implicit*/ uniform_int_distribution(const std_type& distribution)
            : uniform_int_distribution(distribution.a(), distribution.b())
        {
        } // uniform_int_distribution(...)

        /** @exception std::logic_error \p min exceeds \p max. */
        explicit uniform_int_distribution(value_type min, value_type max)
            : m_min(min), m_max(max)
        {
            this->validate();
        } // uniform_int_distribution(...)

        /** Converts the distribution to its standard built-in counterpart. */
        std_type to_std() const noexcept
        {
            return std_type(this->m_min, this->m_max);
        } // to_std(...)

        /** Smallest value. */
        value_type a() const noexcept { return this->m_min; }
        /** Largest value. */
        value_type b() const noexcept { return this->m_max; }

        /** Expected value of the distribution. */
        expectation_type expected_value() const noexcept
        {
            expectation_type x = static_cast<expectation_type>(this->m_min) / 2;
            expectation_type y = static_cast<expectation_type>(this->m_max) / 2;
            return x + y;
        } // expected_value(...)
        /** Variance of the distribution. */
        expectation_type variance() const noexcept
        {
            expectation_type diam = static_cast<expectation_type>(this->m_max - this->m_min);
            return math_constants<expectation_type>::one_over_twelwe * diam * (diam + 2);
        } // variance(...)
        /** Standard deviation of the distribution. */
        expectation_type standard_deviation() const noexcept { return std::sqrt(this->variance()); }

        /** Expected value of the distribution. */
        expectation_type mean() const noexcept { return this->expected_value(); }
        /** Standard deviation of the distribution. */
        expectation_type stddev() const noexcept { return this->standard_deviation(); }
        /** Smallest value in the distribution. */
        value_type min() const noexcept { return this->m_min; }
        /** Largest value in the distribution. */
        value_type max() const noexcept { return this->m_max; }

        /** Cumulative distribution function (c.d.f.) of the distribution. */
        probability_type cdf(value_type k) const noexcept
        {
            if (k < this->m_min) return 0;
            if (k >= this->m_max) return 1;
            k -= this->m_min;
            probability_type diam = static_cast<probability_type>(this->m_max - this->m_min);
            return static_cast<probability_type>(k + 1) / (diam + 1);
        } // cdf(...)

        /** Point mass function (p.m.f.) of the distribution. */
        probability_type pmf(value_type k, probability_type scale = 1) const noexcept
        {
            if (k < this->m_min) return 0;
            if (k > this->m_max) return 0;
            probability_type diam = static_cast<probability_type>(this->m_max - this->m_min);
            return scale / (diam + 1);
        } // pmf(...)

        /** @brief Support of the distribution.
         *  @exception std::overflow_error Support too large to be stored in memory.
         */
        std::vector<value_type> support() const
        {
            std::size_t diam = static_cast<std::size_t>(this->m_max - this->m_min);
            if (diam + 1 < diam) throw std::overflow_error("Support too large to be stored in memory.");
            std::vector<value_type> result {};
            result.reserve(diam + 1);
            for (value_type k = this->m_min; k <= this->m_max; ++k) result.push_back(k);
            return result;
        } // support(...)

        /** Checks if the two distributions are the same. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_min == other.m_min &&
                this->m_max == other.m_max;
        } // operator ==(...)

        /** Checks if the two distributions are different. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };

            static type default_instance {};

            if (x.m_min != default_instance.m_min) j[std::string(type::jstr_min)] = x.m_min;
            if (x.m_max != default_instance.m_max) j[std::string(type::jstr_max)] = x.m_max;
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <uniform_int_distribution> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct uniform_int_distribution
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
            if (!noexcept_json::optional(j, result_type::jstr_min, x.m_min)) return false;
            if (!noexcept_json::optional(j, result_type::jstr_max, x.m_max)) return false;

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

            std::hash<typename argument_type::mu_type> min_hasher = {};
            std::hash<typename argument_type::mu_type> max_hasher = {};

            result ^= (min_hasher(x.m_min) << ((shift * 0) % total_width));
            result ^= (max_hasher(x.m_max) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_PROBABILITY_UNIFORM_INT_DISTRIBUTION_HPP_INCLUDED
