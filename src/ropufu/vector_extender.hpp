
#ifndef ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED

#include <cstddef>     // std::size_t
#include <algorithm>   // std::sort
#include <ranges>      // std::ranges::...
#include <type_traits> // std::decay_t, std::type_identity_t
#include <utility>     // std::declval, std::swap
#include <vector>      // std::vector

namespace ropufu
{
    template <std::ranges::random_access_range t_container_type>
    inline void sort(t_container_type& collection) noexcept
    {
        std::sort(collection.begin(), collection.end());
    } // sort(...)

    template <std::ranges::random_access_range t_container_type, typename t_comparer_type>
    inline void sort(t_container_type& collection, t_comparer_type comparer) noexcept
    {
        std::sort(collection.begin(), collection.end(), comparer);
    } // sort(...)

    template <std::ranges::random_access_range t_container_type>
    inline void fill(t_container_type& collection, const std::type_identity_t<std::ranges::range_value_t<t_container_type>>& value) noexcept
    {
        for (auto& x : collection) x = value;
    } // fill(...)

    template <std::ranges::random_access_range t_container_type>
    inline bool contains(const t_container_type& collection, const std::type_identity_t<std::ranges::range_value_t<t_container_type>>& value) noexcept
    {
        for (const auto& x : collection) if (x == value) return true;
        return false;
    } // contains(...)

    template <typename t_value_type, typename t_allocator_type>
    void append_distinct(std::vector<t_value_type, t_allocator_type>& left, const std::vector<t_value_type, t_allocator_type>& right) noexcept
    {
        for (const t_value_type& x : right)
        {
            if (!ropufu::contains(left, x)) left.push_back(x);
        } // for (...)
    } // append_distinct(...)

    template <typename t_value_type, typename t_allocator_type>
    std::vector<t_value_type, t_allocator_type> distinct(const std::vector<t_value_type, t_allocator_type>& collection) noexcept
    {
        std::vector<t_value_type, t_allocator_type> result {};
        result.reserve(collection.size());
        ropufu::append_distinct(result, collection);
        return result;
    } // distinct(...)

    template <typename t_value_type, typename t_allocator_type>
    std::vector<t_value_type, t_allocator_type> union_of(const std::vector<t_value_type, t_allocator_type>& a, const std::vector<t_value_type, t_allocator_type>& b) noexcept
    {
        std::vector<t_value_type, t_allocator_type> result {};
        result.reserve(a.size() + b.size());
        ropufu::append_distinct(result, a);
        ropufu::append_distinct(result, b);
        return result;
    } // union_of(...)

    template <typename t_value_type, typename t_allocator_type>
    inline std::vector<t_value_type, t_allocator_type> sample(const std::vector<t_value_type, t_allocator_type>& collection, const std::vector<std::size_t>& indices) noexcept
    {
        std::vector<t_value_type, t_allocator_type> result {};
        result.reserve(indices.size());
        for (std::size_t j : indices) result.push_back(collection[j]);
        return result;
    } // sample(...)
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
