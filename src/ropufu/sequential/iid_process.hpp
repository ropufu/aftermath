
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"

#include "scalar_process.hpp"

#include <chrono>      // std::chrono::system_clock
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <random>      // std::seed_seq
#include <ranges>      // std::ranges::...
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME iid_process<t_engine_type, t_sampler_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <typename t_engine_type, typename t_sampler_type> \
                                      requires std::totally_ordered<typename t_sampler_type::value_type>

namespace ropufu::aftermath::sequential
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct iid_process;

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct iid_process : public scalar_process<typename t_sampler_type::value_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = scalar_process<typename t_sampler_type::value_type>;
        using engine_type = t_engine_type;
        using sampler_type = t_sampler_type;

	    using distribution_type = typename sampler_type::distribution_type;
        using value_type = typename sampler_type::value_type;

        static constexpr std::string_view name = "iid";
        static constexpr std::size_t parameter_dim = 1;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_distribution = "distribution";
        
        friend ropufu::noexcept_json_serializer<type>;
        friend std::hash<type>;

    private:
        engine_type m_engine;
        sampler_type m_sampler;
        /** @todo Replace with parameter struct. */
        distribution_type m_distribution;

        static engine_type make_engine() noexcept
        {
            engine_type result{};
            int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
            std::seed_seq sequence{ 1, 1, 2, 3, 5, 8, 1729, time_seed };
            result.seed(sequence);
            return result;
        } // make_engine(...)

    protected:
        void on_clear() noexcept override { }

        value_type on_next() noexcept override
        {
            return this->m_sampler(this->m_engine);
        } // on_next(...)

        template <std::ranges::random_access_range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        virtual void on_next(t_container_type& values) noexcept override
        {
            for (value_type& x : values) x = this->m_sampler(this->m_engine);
        } // on_next(...)

    public:
        iid_process() noexcept
            : iid_process(distribution_type{})
        {
        } // iid_process(...)

        explicit iid_process(const distribution_type& dist) noexcept
            : m_engine(type::make_engine()), m_sampler(dist), m_distribution(dist)
        {
        } // iid_process(...)

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
    }; // struct iid_process
} // namespace ropufu::aftermath::sequential

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
