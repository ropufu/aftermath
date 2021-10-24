
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../simple_vector.hpp"
#include "discrete_process.hpp"

#include <chrono>      // std::chrono::system_clock
#include <concepts>    // std::same_as, std::totally_ordered
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <random>      // std::seed_seq
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::logic_error
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME iid_persistent_process<t_no_change_sampler_type, t_under_change_sampler_type, t_container_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE \
    template <typename t_no_change_sampler_type, typename t_under_change_sampler_type, std::ranges::random_access_range t_container_type> \
    requires \
        std::totally_ordered<typename t_no_change_sampler_type::value_type> && \
        std::same_as<typename t_no_change_sampler_type::value_type, typename t_under_change_sampler_type::value_type> && \
        std::same_as<typename t_no_change_sampler_type::value_type, std::ranges::range_value_t<t_container_type>>

namespace ropufu::aftermath::sequential
{
    /** Sequence of independent observations with two modes: no-change iid, under-change iid. */
    template <typename t_no_change_sampler_type, typename t_under_change_sampler_type,
        std::ranges::random_access_range t_container_type = aftermath::simple_vector<typename t_no_change_sampler_type::value_type>>
    requires
        std::totally_ordered<typename t_no_change_sampler_type::value_type> &&
        std::same_as<typename t_no_change_sampler_type::value_type, typename t_under_change_sampler_type::value_type> &&
        std::same_as<typename t_no_change_sampler_type::value_type, std::ranges::range_value_t<t_container_type>>
    struct iid_persistent_process;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct iid_persistent_process : public discrete_process<typename t_no_change_sampler_type::value_type, t_container_type>
    {
        using type = ROPUFU_TMP_TYPENAME;
        using base_type = discrete_process<typename t_no_change_sampler_type::value_type, t_container_type>;
        using no_change_sampler_type = t_no_change_sampler_type;
        using under_change_sampler_type = t_under_change_sampler_type;
        using container_type = t_container_type;

        using no_change_engine_type = typename no_change_sampler_type::engine_type;
        using under_change_engine_type = typename under_change_sampler_type::engine_type;
	    using no_change_distribution_type = typename no_change_sampler_type::distribution_type;
	    using under_change_distribution_type = typename under_change_sampler_type::distribution_type;
        using value_type = typename no_change_sampler_type::value_type;

        static constexpr std::string_view name = "iid persistent";
        static constexpr std::size_t parameter_dim = 3;
        
        // ~~ Json names ~~
        static constexpr std::string_view jstr_type = "type";
        static constexpr std::string_view jstr_no_change_distribution = "no-change distribution";
        static constexpr std::string_view jstr_under_change_distribution = "under-change distribution";
        static constexpr std::string_view jstr_first_under_change_index = "first under-change index";
        
#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;

    private:
        no_change_engine_type m_no_change_engine;
        under_change_engine_type m_under_change_engine;
        no_change_sampler_type m_no_change_sampler;
        under_change_sampler_type m_under_change_sampler;
        std::size_t m_first_under_change_index;
        /** @todo Replace with parameter struct. */
        no_change_distribution_type m_no_change_distribution;
        /** @todo Replace with parameter struct. */
        under_change_distribution_type m_under_change_distribution;

        template <typename t_engine_type>
        static t_engine_type make_engine() noexcept
        {
            t_engine_type result{};
            int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
            std::seed_seq sequence{ 0, 1, 3, 6, 10, 15, 1729, time_seed };
            result.seed(sequence);
            return result;
        } // make_engine(...)

    protected:
        void on_clear() noexcept override { }

        value_type on_next() noexcept override
        {
            std::size_t count = this->count();
            return (count >= this->m_first_under_change_index) ?
                this->m_under_change_sampler(this->m_under_change_engine) : this->m_no_change_sampler(this->m_no_change_engine);
        } // on_next(...)

        void on_next(container_type& values) noexcept override
        {
            std::size_t count = this->count();
            std::size_t length = std::ranges::size(values);
            // @todo Consider hashing the change passed/in progress flags.
            if (count >= this->m_first_under_change_index) // Change is in effect.
            {
                for (value_type& x : values) x = this->m_under_change_sampler(this->m_under_change_engine);
            } // if (...)
            else if (count + length <= this->m_first_under_change_index) // Change is in the distant future.
            {
                for (value_type& x : values) x = this->m_no_change_sampler(this->m_no_change_engine);
            } // if (...)
            else
            {
                // Two blocks: pre-change, under-change.
                std::size_t pre_size = (this->m_first_under_change_index > count) ? this->m_first_under_change_index - count : 0;
                for (std::size_t i = 0; i < pre_size; ++i) values[i] = this->m_no_change_sampler(this->m_no_change_engine);
                for (std::size_t i = pre_size; i < length; ++i) values[i] = this->m_under_change_sampler(this->m_under_change_engine);
            } // else
        } // on_next(...)

    public:
        iid_persistent_process() noexcept
            : iid_persistent_process(no_change_distribution_type{}, under_change_distribution_type{}, 0)
        {
        } // iid_persistent_process(...)

        iid_persistent_process(const no_change_distribution_type& no_change_dist, const under_change_distribution_type& under_change_dist,
            std::size_t first_under_change_index) noexcept
            : m_no_change_engine(type::make_engine<no_change_engine_type>()), m_under_change_engine(type::make_engine<under_change_engine_type>()),
            m_no_change_sampler(no_change_dist), m_under_change_sampler(under_change_dist),
            m_first_under_change_index(first_under_change_index),
            m_no_change_distribution(no_change_dist), m_under_change_distribution(under_change_dist)
        {
        } // iid_persistent_process(...)

        std::size_t first_under_change_index() const noexcept { return this->m_first_under_change_index; }

        /** Check for parameter equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_no_change_distribution == other.m_no_change_distribution &&
                this->m_under_change_distribution == other.m_under_change_distribution &&
                this->m_first_under_change_index == other.m_first_under_change_index;
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
                {type::jstr_no_change_distribution, x.m_no_change_distribution},
                {type::jstr_under_change_distribution, x.m_under_change_distribution},
                {type::jstr_first_under_change_index, x.m_first_under_change_index}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <iid_persistent_process> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct iid_persistent_process
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
            if (!noexcept_json::required(j, result_type::jstr_no_change_distribution, x.m_no_change_distribution)) return false;
            if (!noexcept_json::required(j, result_type::jstr_under_change_distribution, x.m_under_change_distribution)) return false;
            if (!noexcept_json::required(j, result_type::jstr_first_under_change_index, x.m_first_under_change_index)) return false;

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

            std::hash<typename argument_type::no_change_distribution_type> no_change_distribution_hasher = {};
            std::hash<typename argument_type::under_change_distribution_type> under_change_distribution_hasher = {};
            std::hash<std::size_t> first_under_change_index_hasher = {};

            result ^= (no_change_distribution_hasher(x.m_no_change_distribution) << ((shift * 0) % total_width));
            result ^= (under_change_distribution_hasher(x.m_under_change_distribution) << ((shift * 1) % total_width));
            result ^= (first_under_change_index_hasher(x.m_first_under_change_index) << ((shift * 2) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std
#endif // ROPUFU_AFTERMATH_SEQUENTIAL_IID_PERSISTENT_PROCESS_HPP_INCLUDED
