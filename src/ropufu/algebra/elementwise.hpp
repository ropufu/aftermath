
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include "../concepts.hpp"
#include "../simple_vector.hpp"

#include <algorithm>  // std::sort
#include <functional> // std::less, std::greater
#include <limits>     // std::numeric_limits
#include <ranges>     // std::ranges
#include <vector>     // std::vector

namespace ropufu::aftermath::algebra
{
    /** Makes \param container the identity permutation. */
    template <std::ranges::random_access_range t_container_type>
    static bool is_permutation(t_container_type& container) noexcept
    {
        using size_type = std::ranges::range_value_t<t_container_type>;

        size_type n = container.size();
        aftermath::simple_vector<bool> values(n);
        for (size_type i = 0; i < n; ++i)
        {
            size_type x = container[i];

            if (x >= n) return false;
            if constexpr (std::numeric_limits<size_type>::is_signed) if (x < 0) return false;

            values[x] = true;
        } // for (...)

        for (size_type i = 0; i < n; ++i) if (values[i] == false) return false;
        return true;
    } // is_permutation(...)

    /** An identity permutation with \p size many elements. */
    template <typename t_size_type>
    static std::vector<t_size_type> identity_permutation(t_size_type size) noexcept
    {
        std::vector<t_size_type> result {};
        result.reserve(size);
        for (t_size_type i = 0; i < size; ++i) result.push_back(i);
        result.shrink_to_fit();
        return result;
    } // identity_permutation(...)

    /** Makes \param container the identity permutation. */
    template <std::ranges::random_access_range t_container_type>
    static void make_identity_permutation(t_container_type& container) noexcept
    {
        using size_type = std::ranges::range_value_t<t_container_type>;
        for (size_type i = 0; i < container.size(); ++i) container[i] = i;
    } // make_identity_permutation(...)

    /** Inverts \param permutation.
     *  @warning Does not perform any validation.
     */
    template <std::ranges::random_access_range t_container_type>
    static t_container_type inverse_permutation(const t_container_type& permutation)
    {
        using size_type = std::ranges::range_value_t<t_container_type>;
        t_container_type result(permutation);
        for (size_type i = 0; i < result.size(); ++i) result[permutation[i]] = i;
        return result;
    } // inverse_permutation(...)

    /** @warning Does not perform any validation. */
    template <std::ranges::random_access_range t_container_type>
    static t_container_type compose_permutations(const t_container_type& first, const t_container_type& second)
    {
        using size_type = std::ranges::range_value_t<t_container_type>;
        t_container_type result(first);
        for (size_type i = 0; i < result.size(); ++i) result[i] = first[second[i]];
        return result;
    } // compose_permutations(...)

    /** Permutation to allow subscript access to the elements of \p source in the order specified by the \p comparer. */
    template <std::ranges::sized_range t_container_type, typename t_comparer_type>
        requires ropufu::binary_predicate<t_comparer_type, const typename t_container_type::value_type&, const typename t_container_type::value_type&>
    static std::vector<std::ranges::range_size_t<t_container_type>> permutation(const t_container_type& source, t_comparer_type&& comparer)
    {
        using size_type = std::ranges::range_size_t<t_container_type>;
        std::vector<size_type> result = algebra::identity_permutation(source.size());
        std::sort(result.begin(), result.end(),
            [&source, &comparer] (size_type i, size_type j) {
                return comparer(source[i], source[j]);
        }); // std::sort(...)
        return result;
    } // permutation(...)
    
    /** Permutation describing element access in ascending order. */
    template <std::ranges::sized_range t_container_type>
    static std::vector<std::ranges::range_size_t<t_container_type>> ascending_permutation(const t_container_type& source) noexcept
    {
        return algebra::permutation(source, std::less<typename t_container_type::value_type>());
    } // ascending_permutation(...)
    
    /** Permutation describing element access in descending order. */
    template <std::ranges::sized_range t_container_type>
    static std::vector<std::ranges::range_size_t<t_container_type>> descending_permutation(const t_container_type& source) noexcept
    {
        return algebra::permutation(source, std::greater<typename t_container_type::value_type>());
    } // descending_permutation(...)

    /** @brief Modifies \p source by subtracting every element of \p other from it. */
    template <std::ranges::range t_source_container_type, std::ranges::range t_target_container_type>
    static bool try_subtract_assign(t_source_container_type& source, const t_target_container_type& other) noexcept
    {
        auto source_it = source.begin();
        auto other_it = other.cbegin();
        while (source_it != source.end() && other_it != other.cend())
        {
            (*source_it) -= (*other_it);
            ++source_it;
            ++other_it;
        } // while (...)
        return source_it == source.end() && other_it == other.cend();
    } // try_subtract_assign(...)

    /** @brief Modifies \p source by adding every element of \p other to it. */
    template <std::ranges::range t_source_container_type, std::ranges::range t_target_container_type>
    static bool try_add_assign(t_source_container_type& source, const t_target_container_type& other) noexcept
    {
        auto source_it = source.begin();
        auto other_it = other.cbegin();
        while (source_it != source.end() && other_it != other.cend())
        {
            (*source_it) += (*other_it);
            ++source_it;
            ++other_it;
        } // while (...)
        return source_it == source.end() && other_it == other.cend();
    } // try_add_assign(...)

    /** @brief Computes a binary mask for elements of \p source indicated by \p predicate.
     *  @param source The container of the elements to filter.
     *  @param predicate Predicate indicating which elements to select. Has to implement (const t_container_type::value_type&) -> bool.
     *  @remark \tparam t_integral_mask_type has to implement operator << and operator |.
     */
    template <std::ranges::range t_container_type, typename t_predicate_type, ropufu::binary_mask t_integral_mask_type>
        requires ropufu::zero_assignable<t_integral_mask_type> &&
            ropufu::one_assignable<t_integral_mask_type> &&
            ropufu::unary_predicate<t_predicate_type, const typename t_container_type::value_type&>
    static void to_binary_mask(const t_container_type& source, t_predicate_type&& predicate, t_integral_mask_type& result) noexcept
    {
        result = 0;
        t_integral_mask_type flag = 1;
        for (const auto& x : source)
        {
            if (predicate(x)) result |= flag;
            flag <<= 1;
        } // for (...)
    } // to_binary_mask(...)

    /** @brief Adds up elements of \p source marked by \p mask.
     *  @param source The container of the elements to \p result.
     *  @param mask Binary mask indicating which elements to \p result.
     *  @remark \tparam t_integral_mask_type has to implement operator >>.
     */
    template <std::ranges::range t_container_type, ropufu::binary_mask t_integral_mask_type, typename t_value_type>
        requires ropufu::zero_assignable<t_value_type>
    static void masked_sum(const t_container_type& source, t_integral_mask_type mask, t_value_type& result) noexcept
    {
        result = 0;
        for (const auto& x : source)
        {
            if ((mask & 1) == 1) result += x;
            mask >>= 1;
        } // for (...)
    } // masked_sum(...)

    /** @brief Performs the non-invasive \p action for every elements of \p source marked by \p mask.
     *  @param source The container of the elements to select from.
     *  @param mask Binary mask indicating which elements to select.
     *  @param action Has to implement (const t_container_type::value_type&) -> void.
     *  @remark \tparam t_integral_mask_type has to implement operator >>.
     */
    template <std::ranges::range t_container_type, ropufu::binary_mask t_integral_mask_type, typename t_action_type>
        requires ropufu::unary_action<t_action_type, const typename t_container_type::value_type&>
    static void masked_touch(const t_container_type& source, t_integral_mask_type mask, t_action_type&& action) noexcept
    {
        for (const auto& x : source)
        {
            if ((mask & 1) == 1) action(x);
            mask >>= 1;
        } // for (...)
    } // masked_touch(...)

    /** @brief Performs the potentially invasive \p action for every elements of \p source marked by \p mask.
     *  @param source The container of the elements to select from.
     *  @param mask Binary mask indicating which elements to select.
     *  @param action Has to implement (t_container_type::value_type&) -> void.
     *  @remark \tparam t_integral_mask_type has to implement operator >>.
     */
    template <std::ranges::range t_container_type, ropufu::binary_mask t_integral_mask_type, typename t_action_type>
        requires ropufu::unary_action<t_action_type, typename t_container_type::value_type&>
    static void masked_action(t_container_type& source, t_integral_mask_type mask, t_action_type&& action) noexcept
    {
        for (auto& x : source)
        {
            if ((mask & 1) == 1) action(x);
            mask >>= 1;
        } // for (...)
    } // masked_action(...)
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
