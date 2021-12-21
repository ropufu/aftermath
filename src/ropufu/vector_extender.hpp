
#ifndef ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "noexcept_json.hpp"
#include "algebra/interval_spacing.hpp"
#include "algebra/interval_based_vector.hpp"
#endif

#include "concepts.hpp"

#include <concepts>    // std::same_as
#include <cstddef>     // std::size_t
#include <ranges>      // std::ranges::...
#include <type_traits> // std::decay_t, std::type_identity_t
#include <utility>     // std::declval, std::swap
#include <variant>     // std::variant, std::monostate, std::visit

namespace ropufu
{
    template <std::ranges::range t_container_type>
    void fill(t_container_type& collection, const std::type_identity_t<std::ranges::range_value_t<t_container_type>>& value) noexcept
    {
        for (auto& x : collection) x = value;
    } // fill(...)

    template <std::ranges::range t_container_type>
    bool contains(const t_container_type& collection, const std::type_identity_t<std::ranges::range_value_t<t_container_type>>& value) noexcept
    {
        for (const auto& x : collection) if (x == value) return true;
        return false;
    } // contains(...)

    template <push_back_container t_result_container_type, std::ranges::range t_container_type>
    void append_distinct(t_result_container_type& left, const t_container_type& right) noexcept
    {
        for (const auto& x : right)
        {
            if (!contains(left, x)) left.push_back(x);
        } // for (...)
    } // append_distinct(...)

    template <push_back_container t_result_container_type, std::ranges::random_access_range t_container_type, std::ranges::range t_index_container_type>
    void append_sample(t_result_container_type& result, const t_container_type& collection, const t_index_container_type& indices) noexcept
    {
        for (const auto& j : indices) result.push_back(collection[j]);
    } // sample(...)
} // namespace ropufu

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    template <spacing... t_spacing_types>
        requires all_same<typename t_spacing_types::value_type...>
    using vector_initializer_t = std::variant<
            std::monostate,
            aftermath::algebra::interval_based_vector<t_spacing_types>...>;

    template <push_back_container t_container_type, spacing... t_spacing_types>
        requires all_same<
            std::ranges::range_value_t<t_container_type>,
            typename t_spacing_types::value_type...>
    bool try_serialize(nlohmann::json& j, const t_container_type& container, const vector_initializer_t<t_spacing_types...>& initializer) noexcept
    {
        if (initializer.index() == 0) j = container;
        else
            std::visit([&j] (auto&& arg) {
                using arg_type = std::decay_t<decltype(arg)>;
                if constexpr (!std::same_as<arg_type, std::monostate>) j = arg;
            }, initializer);
        return true;
    } // try_serialize(...)

    template <spacing... t_spacing_types, push_back_container t_container_type>
        requires all_same<
            std::ranges::range_value_t<t_container_type>,
            typename t_spacing_types::value_type...>
    bool try_deserialize(const nlohmann::json& j, t_container_type& container) noexcept
    {
        if (j.is_array()) return noexcept_json::try_get(j, container);
        
        vector_initializer_t<t_spacing_types...> initializer {};
        if (!noexcept_json::try_get(j, initializer)) return false;

        std::visit([&container] (auto&& arg) {
            using arg_type = std::decay_t<decltype(arg)>;
            if constexpr (!std::same_as<arg_type, std::monostate>) arg.explode(container);
        }, initializer);

        return true;
    } // try_deserialize(...)
} // namespace ropufu
#endif

#endif // ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
