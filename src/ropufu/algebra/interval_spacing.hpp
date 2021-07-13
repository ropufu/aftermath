
#ifndef ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED

#include <cmath>       // std::log, std::pow
#include <concepts>    // std::same_as
#include <cstddef>     // std::size_t
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

    private:
        intermediate_type m_log_base = std::numbers::e_v<intermediate_type>;
        intermediate_type m_log_factor = 1;

    public:
        logarithmic_spacing() noexcept { }

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
    }; // struct exponential_spacing
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_SPACING_HPP_INCLUDED
