
#ifndef ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include <cmath>       // std::log, std::pow
#include <concepts>    // std::same_as
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <numbers>     // std::numbers::e_v
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::declval

namespace ropufu
{
    template <typename t_type>
    concept spacing = requires(const t_type& x, std::string& s)
    {
        typename t_type::value_type;
        typename t_type::intermediate_type;
        requires !t_type::name.empty();

        s = t_type::name;

        /** Sends data points to where they are linearly spaced. */
        {x.forward_transform(std::declval<typename t_type::value_type>())}
            -> std::same_as<typename t_type::intermediate_type>;
        /** Sends transformed points back to to where they came from. */
        {x.backward_transform(std::declval<typename t_type::intermediate_type>())}
            -> std::same_as<typename t_type::value_type>;
    }; // concept spacing
} // namespace ropufu

namespace ropufu::aftermath::algebra
{
    /** Intermediate points in [a, b] will be spaced equidistantly.
     *  Specifically, for i = 0, 1, ..., n, the i-th point will have
     *  the form a + i (b - a) / n.
     *  @example [1, 2, 3, 4, 5] is a linear spacing of [1, 5].
     */
    template <typename t_value_type, typename t_intermediate_type = t_value_type>
    struct linear_spacing
    {
        using type = linear_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;

        static constexpr std::string_view name = "linear";

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

        constexpr linear_spacing() noexcept { }

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return value;
            else return static_cast<intermediate_type>(value);
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return transformed_value;
            else return static_cast<value_type>(transformed_value);
        } // backward_transform(...)

        constexpr bool operator ==(const type& /*other*/) const noexcept
        {
            return true;
        } // operator ==(...)

        constexpr bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& /*x*/) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <linear_spacing> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct linear_spacing

    /** Intermediate points in [a, b] will be spaced logarithmically.
     *  Specifically, for i = 0, 1, ..., n, the i-th point will have
     *  the form a (b / a)^(i / n).
     *  @remark When constructing intervals based on log-spacing, the
     *  choice of base should not matter.
     *  @example [1, 1.5, 2.2, 3.3, 5] is a logarithmic spacing of [1, 5].
     */
    template <typename t_value_type, typename t_intermediate_type = double>
    struct logarithmic_spacing
    {
        using type = logarithmic_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;
        
        static constexpr std::string_view name = "logarithmic";
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        intermediate_type m_log_base = std::numbers::e_v<intermediate_type>;
        intermediate_type m_log_factor = 1;

    public:
        constexpr logarithmic_spacing() noexcept { }

        explicit logarithmic_spacing(intermediate_type log_base) noexcept
            : m_log_base(log_base), m_log_factor(1 / std::log(log_base))
        { }

        const intermediate_type& log_base() const noexcept { return this->m_log_base; }

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return this->m_log_factor * std::log(value);
            else return this->m_log_factor * std::log(static_cast<intermediate_type>(value));
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return std::pow(this->m_log_base, transformed_value);
            else return static_cast<value_type>(std::pow(this->m_log_base, transformed_value));
        } // backward_transform(...)

        constexpr bool operator ==(const type& /*other*/) const noexcept
        {
            return true;
        } // operator ==(...)

        constexpr bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& /*x*/) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <logarithmic_spacing> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct logarithmic_spacing

    /** Intermediate points in [a, b] will be spaced exponentially.
     *  Specifically, for i = 0, 1, ..., n, the i-th point will have
     *  the form log(exp(a) + i (exp(b) - exp(a)) / n), where
     *  \li log(x) = ln(x) / ln(base)
     *  \li exp(x) = (base) to the power x
     *  @example [1, 3.7, 4.3, 4.7, 5] is an exponential spacing of [1, 5].
     */
    template <typename t_value_type, typename t_intermediate_type = double>
    struct exponential_spacing
    {
        using type = exponential_spacing<t_value_type, t_intermediate_type>;
        using value_type = t_value_type;
        using intermediate_type = t_intermediate_type;
        
        static constexpr std::string_view name = "exponential";
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_log_base = "base";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        intermediate_type m_log_base = std::numbers::e_v<intermediate_type>;
        intermediate_type m_log_factor = 1;

    public:
        exponential_spacing() noexcept { }

        explicit exponential_spacing(intermediate_type log_base) noexcept
            : m_log_base(log_base), m_log_factor(1 / std::log(log_base))
        { }

        const intermediate_type& log_base() const noexcept { return this->m_log_base; }

        /** Sends data points to where they are linearly spaced. */
        intermediate_type forward_transform(const value_type& value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return std::pow(this->m_log_base, value);
            else return std::pow(this->m_log_base, static_cast<intermediate_type>(value));
        } // forward_transform(...)

        /** Sends transformed points back to to where they came from. */
        value_type backward_transform(const intermediate_type& transformed_value) const noexcept
        {
            if constexpr (std::same_as<intermediate_type, value_type>) return this->m_log_factor * std::log(transformed_value);
            else return static_cast<value_type>(this->m_log_factor * std::log(transformed_value));
        } // backward_transform(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_log_factor == other.m_log_factor;
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
                {type::jstr_log_base, x.m_log_base}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <exponential_spacing> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct exponential_spacing
} // namespace ropufu::aftermath::algebra

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    template <typename t_value_type, typename t_intermediate_type>
    struct noexcept_json_serializer<ropufu::aftermath::algebra::linear_spacing<t_value_type, t_intermediate_type>>
    {
        using result_type = ropufu::aftermath::algebra::linear_spacing<t_value_type, t_intermediate_type>;
        static bool try_get(const nlohmann::json& j, result_type& /*x*/) noexcept
        {
            if (j.is_string()) return j.get<std::string>() == result_type::name;

            std::string name;
            if (!noexcept_json::required(j, result_type::jstr_type, name)) return false;
            if (name != result_type::name) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
    
    template <typename t_value_type, typename t_intermediate_type>
    struct noexcept_json_serializer<ropufu::aftermath::algebra::logarithmic_spacing<t_value_type, t_intermediate_type>>
    {
        using result_type = ropufu::aftermath::algebra::logarithmic_spacing<t_value_type, t_intermediate_type>;
        static bool try_get(const nlohmann::json& j, result_type& /*x*/) noexcept
        {
            if (j.is_string()) return j.get<std::string>() == result_type::name;
            
            std::string name;
            if (!noexcept_json::required(j, result_type::jstr_type, name)) return false;
            if (name != result_type::name) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
    
    template <typename t_value_type, typename t_intermediate_type>
    struct noexcept_json_serializer<ropufu::aftermath::algebra::exponential_spacing<t_value_type, t_intermediate_type>>
    {
        using result_type = ropufu::aftermath::algebra::exponential_spacing<t_value_type, t_intermediate_type>;
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (j.is_string()) return j.get<std::string>() == result_type::name;
            
            std::string name;
            if (!noexcept_json::required(j, result_type::jstr_type, name)) return false;
            if (!noexcept_json::required(j, result_type::jstr_log_base, x.m_log_base)) return false;
            if (name != result_type::name) return false;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

namespace std
{
    template <typename t_value_type, typename t_intermediate_type>
    struct hash<ropufu::aftermath::algebra::linear_spacing<t_value_type, t_intermediate_type>>
    {
        using argument_type = ropufu::aftermath::algebra::linear_spacing<t_value_type, t_intermediate_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            return 0;
        } // operator ()(...)
    }; // struct hash<...>

    template <typename t_value_type, typename t_intermediate_type>
    struct hash<ropufu::aftermath::algebra::logarithmic_spacing<t_value_type, t_intermediate_type>>
    {
        using argument_type = ropufu::aftermath::algebra::logarithmic_spacing<t_value_type, t_intermediate_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            return 0;
        } // operator ()(...)
    }; // struct hash<...>

    template <typename t_value_type, typename t_intermediate_type>
    struct hash<ropufu::aftermath::algebra::exponential_spacing<t_value_type, t_intermediate_type>>
    {
        using argument_type = ropufu::aftermath::algebra::exponential_spacing<t_value_type, t_intermediate_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_value_type> value_hash {};
            return value_hash(x.m_log_base);
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED
