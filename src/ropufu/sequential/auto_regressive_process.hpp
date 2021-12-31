
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../number_traits.hpp"
#include "../simple_vector.hpp"
#include "../sliding_array.hpp"
#include "discrete_process.hpp"

#include <concepts>    // std::same_as, std::floating_point
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <optional>    // std::optional, std::nullopt
#include <random>      // std::seed_seq
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error, std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME auto_regressive_process<t_sampler_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE                                                                       \
    template <typename t_sampler_type, std::ranges::random_access_range t_container_type>                   \
        requires                                                                                            \
            std::floating_point<typename t_sampler_type::value_type> &&                                     \
            std::same_as<std::ranges::range_value_t<t_container_type>, typename t_sampler_type::value_type> \


namespace ropufu::aftermath::sequential
{
    /** Auto-regressive process driven by an arbitrary distribution. */
    template <typename t_sampler_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<typename t_sampler_type::value_type>>
        requires
            std::floating_point<typename t_sampler_type::value_type> &&
            std::same_as<std::ranges::range_value_t<t_container_type>, typename t_sampler_type::value_type>
    struct auto_regressive_process;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif
    
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct auto_regressive_process : public discrete_process<typename t_sampler_type::value_type, t_container_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = discrete_process<typename t_sampler_type::value_type, t_container_type>;
        using sampler_type = t_sampler_type;
        using container_type = t_container_type;

        using engine_type = typename sampler_type::engine_type;
	    using distribution_type = typename sampler_type::distribution_type;
        using value_type = typename sampler_type::value_type;
        using history_type = ropufu::aftermath::sliding_vector<value_type>;

        static constexpr std::string_view name = "autoregression";
        static constexpr std::size_t parameter_dim = 2;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_distribution = "distribution";
        static constexpr std::string_view jstr_ar_parameters = "AR parameters";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        engine_type m_engine;
        sampler_type m_sampler;
        /** @todo Replace with parameter struct. */
        distribution_type m_distribution;
        // Autoregression parameters [phi_1, phi_2, ..., phi_p].
        container_type m_ar_parameters;
        // Collection of most recent observations [X_{t - 1}, X_{t - 2}, ..., X_{t - p}].
        history_type m_history;

        /** @brief Validates the structure and returns an error message, if any. */
        std::optional<std::string> error_message() const noexcept
        {
            for (const value_type& x : this->m_ar_parameters)
                if (!aftermath::is_finite(x)) return "AR parameters must be finite.";

            return std::nullopt;
        } // error_message(...)

        /** @exception std::logic_error Validation failed. */
        void validate() const
        {
            std::optional<std::string> message = this->error_message();
            if (message.has_value()) throw std::logic_error(message.value());
        } // validate(...)

        void initialize() noexcept
        {
            this->m_history = history_type(this->m_ar_parameters.size());
        } // initialize(...)

    protected:
        void on_clear() noexcept override
        {
            this->m_history.wipe();
        } // on_clear(...)

        value_type on_next() noexcept override
        {
            value_type newest = this->m_sampler(this->m_engine);
            
            auto it = this->m_ar_parameters.cbegin();
            for (value_type x : this->m_history)
            {
                newest += x * (*it);
                ++it;
            } // for (...)

            this->m_history.displace_front(newest);
            return newest;
        } // on_next(...)

        void on_next(container_type& values) noexcept override
        {
            for (value_type& x : values) x = this->on_next();
        } // on_next(...)

    public:
        /** Zero process. */
        auto_regressive_process() noexcept
            : auto_regressive_process(distribution_type{}, container_type{})
        {
        } // auto_regressive_process(...)

        /** @exception std::logic_error Elements of \p ar_parameters must be finite.
         */
        explicit auto_regressive_process(const distribution_type& dist, const container_type& ar_parameters)
            : m_engine(), m_sampler(dist), m_distribution(dist), m_ar_parameters(ar_parameters)
        {
            this->validate();
            this->initialize();
        } // auto_regressive_process(...)

        void seed(std::seed_seq& sequence) noexcept
        {
            this->m_engine.seed(sequence);
        } // seed(...)

        const container_type& ar_parameters() const noexcept { return this->m_ar_parameters; }

        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_distribution == other.m_distribution &&
                this->m_ar_parameters == other.m_ar_parameters;
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
                {type::jstr_distribution, x.m_distribution},
                {type::jstr_ar_parameters, x.m_ar_parameters}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <auto_regressive_process> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct auto_regressive_process
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
            std::string name;

            if (!noexcept_json::required(j, result_type::jstr_type, name)) return false;
            if (!noexcept_json::required(j, result_type::jstr_distribution, x.m_distribution)) return false;
            if (!noexcept_json::required(j, result_type::jstr_ar_parameters, x.m_ar_parameters)) return false;
            
            if (name != result_type::name) return false;
            if (x.error_message().has_value()) return false;
            x.initialize();

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
            constexpr result_type width = total_width / (argument_type::parameter_dim);
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::distribution_type> distribution_hasher = {};
            std::hash<typename argument_type::container_type> ar_parameters_hasher = {};

            result ^= (distribution_hasher(x.m_distribution) << ((shift * 0) % total_width));
            result ^= (ar_parameters_hasher(x.m_ar_parameters) << ((shift * 1) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_AUTO_REGRESSIVE_PROCESS_HPP_INCLUDED
