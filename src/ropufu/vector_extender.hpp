
#ifndef ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED

#include <cstddef>     // std::size_t
#include <algorithm>   // std::sort
#include <type_traits> // std::decay_t
#include <utility>     // std::declval
#include <vector>      // std::vector

namespace ropufu
{
    template <typename t_value_type>
    inline void sort(std::vector<t_value_type>& collection) noexcept
    {
        std::sort(collection.begin(), collection.end());
    } // sort(...)

    template <typename t_value_type, typename t_comparer_type>
    inline void sort(std::vector<t_value_type>& collection, t_comparer_type&& comparer) noexcept
    {
        std::sort(collection.begin(), collection.end(), comparer);
    } // sort(...)

    template <typename t_value_type>
    inline bool contains(std::vector<t_value_type>& collection, const t_value_type& value) noexcept
    {
        for (const t_value_type& x : collection) if (x == value) return true;
        return false;
    } // append_distinct(...)

    template <typename t_value_type>
    void append_distinct(std::vector<t_value_type>& left, const std::vector<t_value_type>& right) noexcept
    {
        for (const t_value_type& x : right)
        {
            if (!ropufu::contains(left, x)) left.push_back(x);
        } // for (...)
    } // append_distinct(...)

    template <typename t_value_type>
    std::vector<t_value_type> distinct(const std::vector<t_value_type>& collection) noexcept
    {
        std::vector<t_value_type> result {};
        result.reserve(collection.size());
        ropufu::append_distinct(result, collection);
        return result;
    } // distinct(...)

    template <typename t_value_type>
    std::vector<t_value_type> union_of(const std::vector<t_value_type>& a, const std::vector<t_value_type>& b) noexcept
    {
        std::vector<t_value_type> result {};
        result.reserve(a.size() + b.size());
        ropufu::append_distinct(result, a);
        ropufu::append_distinct(result, b);
        return result;
    } // distinct(...)

    template <typename t_value_type, typename t_selector_type>
    auto select(const std::vector<t_value_type>& collection, t_selector_type&& selector) noexcept
    {
        using selected_type = std::decay_t<decltype(selector(std::declval<t_value_type>()))>;

        std::vector<selected_type> result {};
        result.reserve(collection.size());
        for (const t_value_type& x : collection) result.push_back(selector(x));
        return result;
    } // select(...)
} // namespace ropufu

#endif // ROPUFU_AFTERMATH_VECTOR_EXTENDER_HPP_INCLUDED
