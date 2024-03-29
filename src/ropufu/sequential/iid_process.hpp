
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../simple_vector.hpp"
#include "discrete_process.hpp"

#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <random>      // std::seed_seq
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::runtime_error
#include <string>      // std::string
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME iid_process<t_sampler_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE                                                                       \
    template <typename t_sampler_type, std::ranges::random_access_range t_container_type>                   \
        requires                                                                                            \
            std::totally_ordered<typename t_sampler_type::value_type> &&                                    \
            std::same_as<std::ranges::range_value_t<t_container_type>, typename t_sampler_type::value_type> \


namespace ropufu::aftermath::sequential
{
    /** Independent identically distributed (iid) sequence of observations. */
    template <typename t_sampler_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<typename t_sampler_type::value_type>>
        requires
            std::totally_ordered<typename t_sampler_type::value_type> &&
            std::same_as<std::ranges::range_value_t<t_container_type>, typename t_sampler_type::value_type>
    struct iid_process;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct iid_process : public discrete_process<typename t_sampler_type::value_type, t_container_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = discrete_process<typename t_sampler_type::value_type, t_container_type>;
        using sampler_type = t_sampler_type;
        using container_type = t_container_type;

        using engine_type = typename sampler_type::engine_type;
	    using distribution_type = typename sampler_type::distribution_type;
        using value_type = typename sampler_type::value_type;

        static constexpr std::string_view name = "iid";
        static constexpr std::size_t parameter_dim = 1;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_distribution = "distribution";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        engine_type m_engine;
        sampler_type m_sampler;
        /** @todo Replace with parameter struct. */
        distribution_type m_distribution;

    protected:
        void on_clear() noexcept override { }

        value_type on_next() noexcept override
        {
            return this->m_sampler(this->m_engine);
        } // on_next(...)

        void on_next(container_type& values) noexcept override
        {
            for (value_type& x : values) x = this->m_sampler(this->m_engine);
        } // on_next(...)

    public:
        iid_process() noexcept
            : iid_process(distribution_type{})
        {
        } // iid_process(...)

        explicit iid_process(const distribution_type& dist) noexcept
            : m_engine(), m_sampler(dist), m_distribution(dist)
        {
        } // iid_process(...)

        void seed(std::seed_seq& sequence) noexcept
        {
            this->m_engine.seed(sequence);
        } // seed(...)

        /** Check for parameter equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_distribution == other.m_distribution;
        } // operator ==(...)

        /** Check for parameter inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !this->operator ==(other);
        } // operator !=(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_type, type::name},
                {type::jstr_distribution, x.m_distribution}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <iid_process> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct iid_process
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

            if (name != result_type::name) return false;

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

            result ^= (distribution_hasher(x.m_distribution) << ((shift * 0) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
