
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_GAUSSIAN_NOISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_GAUSSIAN_NOISE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"

#include "../discrepancy.hpp"
#include "../number_traits.hpp"
#include "../probability/standard_normal_distribution.hpp"
#include "../random/normal_sampler_512.hpp"
#include "scalar_process.hpp"

#include <concepts>    // std::same_as
#include <iostream>    // std::ostream
#include <optional>    // std::optional, std::nullopt
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#define ROPUFU_TMP_TYPENAME gaussian_noise<t_engine_type, t_value_type, t_probability_type, t_expectation_type, t_sampler_type>

namespace ropufu::aftermath::sequential
{
    /** Gaussian noise. */
    template <typename t_engine_type,
        typename t_value_type = typename probability::standard_normal_distribution<>::value_type,
        typename t_probability_type = typename probability::standard_normal_distribution<t_value_type>::probability_type,
        typename t_expectation_type = typename probability::standard_normal_distribution<t_value_type, t_probability_type>::expectation_type,
        typename t_sampler_type = aftermath::random::normal_sampler_512<t_engine_type, t_value_type>>
        requires std::same_as<typename t_sampler_type::distribution_type, typename probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    struct gaussian_noise;

    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);

    /** Gaussian noise. */
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
        requires std::same_as<typename t_sampler_type::distribution_type, typename probability::standard_normal_distribution<t_value_type, t_probability_type, t_expectation_type>>
    struct gaussian_noise : public scalar_process<typename t_sampler_type::value_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = scalar_process<t_value_type>;
        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using probability_type = t_probability_type;
        using expectation_type = t_expectation_type;
        using sampler_type = t_sampler_type;

        using distribution_type = typename sampler_type::distribution_type;

        static constexpr std::string_view name = "gaussian";

        // ~~ Json names ~~
        static constexpr std::string_view jstr_typename = "type";
        static constexpr std::string_view jstr_sigma = "sigma";

        friend ropufu::noexcept_json_serializer<type>;

    private:
        value_type m_sigma = 0; // Standard deviation.
        sampler_type m_sampler = {}; // White noise sampler.
        value_type m_current_value = 0; // Latest observed value.

        std::optional<std::string> error_message() const noexcept
        {
            if (!aftermath::is_finite(this->m_sigma) || this->m_sigma < 0) return "Sigma must be positive or zero.";
            return std::nullopt;
        } // error_message(...)

        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)
        
    public:
        /** @brief Zero Gaussian noise. */
        gaussian_noise() noexcept
        {
        } // gaussian_noise(...)

        /** @brief White Gaussian noise.
         *  @exception std::logic_error \p value must be positive or zero.
         */
        explicit gaussian_noise(value_type sigma)
            : m_sigma(sigma)
        {
            this->validate();
        } // gaussian_noise(...)

        /** @brief Standard deviation of the noise. */
        value_type sigma() const noexcept { return this->m_sigma; }
        /** @brief Standard deviation of the noise. */
        value_type standard_deviation() const noexcept { return this->m_sigma; }
        /** @brief Variance of the noise. */
        value_type variance() const noexcept { return this->m_sigma * this->m_sigma; }
        /** @brief Standard deviation of the noise.
         *  @exception std::logic_error \p value must be positive or zero.
         */
        void set_sigma(value_type value)
        {
            this->m_sigma = value;
            this->validate();
        } // set_sigma(...)

        /** Resets the timer on the noise. */
        void reset() noexcept { this->m_current_value = 0; }

        /** Latest observed value. */
        value_type current_value() const noexcept { return this->m_current_value; }

        void tic(engine_type& uniform_engine) noexcept
        {
            this->m_current_value = this->m_sigma * this->m_sampler(uniform_engine);
        } // tic(...)

        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_sigma == other.m_sigma;
        } // operator ==(...)

        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

        /** Output to a stream. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct gaussian_noise

    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept
    {
        using type = ROPUFU_TMP_TYPENAME;
        std::string typename_str = type::name;

        j = nlohmann::json{
            {type::jstr_typename, typename_str},
            {type::jstr_sigma, x.sigma()}
        };
    } // to_json(...)

    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x)
    {
        if (!noexcept_json::try_get(j, x)) throw std::runtime_error("Parsing <gaussian_noise> failed: " + j.dump());
    } // from_json(...)
} // namespace ropufu::aftermath::sequential

namespace ropufu
{
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
    struct noexcept_json_serializer<ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME>
    {
        using engine_type = t_engine_type;
        using value_type = t_value_type;
        using result_type = ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME;

        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            // Ensure correct type.
            std::string typename_str {};
            if (!noexcept_json::required(j, result_type::jstr_typename, typename_str)) return false;
            if (typename_str != result_type::name) return false; // Noise type mismatch.

            // Parse json entries.
            if (!noexcept_json::optional(j, result_type::jstr_sigma, x.m_sigma)) return false;
            
            // Validate entries.
            if (x.error_message().has_value()) return false;

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace ropufu::aftermath
{
    namespace detail
    {
    template <typename t_engine_type, typename t_value_type, typename t_probability_type, typename t_expectation_type, typename t_sampler_type>
        struct discrepancy<ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME>
        {
            using result_type = t_value_type;
            using argument_type = ropufu::aftermath::sequential::ROPUFU_TMP_TYPENAME;

            result_type operator ()(const argument_type& x, const argument_type& y) const noexcept
            {
                result_type total = 0;
                total += ropufu::aftermath::discrepancy(x.sigma(), y.sigma());
                return total;
            } // operator ()(...)
        }; // struct discrepancy<...>
    } // namespace detail
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_GAUSSIAN_NOISE_HPP_INCLUDED
