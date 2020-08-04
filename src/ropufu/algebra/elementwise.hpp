
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include "../concepts.hpp"

#include <algorithm>  // std::sort
#include <cstddef>    // std::size_t
#include <functional> // std::less, std::greater
#include <ranges>     // std::ranges::range
#include <vector>     // std::vector

namespace ropufu::aftermath::algebra
{
    /** Elementwise actions in containers. */
    struct elementwise
    {
        /** An identity permutation with \p size many elements. */
        static std::vector<std::size_t> identity_permutation(std::size_t size) noexcept
        {
            std::vector<std::size_t> result {};
            result.reserve(size);
            for (std::size_t i = 0; i < size; ++i) result.push_back(i);
            result.shrink_to_fit();
            return result;
        } // identity_permutation(...)

        /** Permutation to allow subscript access to the elements of \p source in the order specified by the \p comparer. */
        template <std::ranges::range t_container_type, typename t_comparer_type>
        static std::vector<std::size_t> permutation(const t_container_type& source, t_comparer_type&& comparer)
        {
            std::vector<std::size_t> result = elementwise::identity_permutation(source.size());
            std::sort(result.begin(), result.end(), 
                [&source, &comparer] (std::size_t i, std::size_t j) {
                    return comparer(source[i], source[j]);
            }); // std::sort(...)
            return result;
        } // try_order_by(...)
        
        /** Permutation describing element access in ascending order. */
        template <std::ranges::range t_container_type>
        static std::vector<std::size_t> ascending_permutation(const t_container_type& source) noexcept
        {
            return elementwise::permutation(source, std::less<typename t_container_type::value_type>());
        } // identity_permutation(...)
        
        /** Permutation describing element access in descending order. */
        template <std::ranges::range t_container_type>
        static std::vector<std::size_t> descending_permutation(const t_container_type& source) noexcept
        {
            return elementwise::permutation(source, std::greater<typename t_container_type::value_type>());
        } // identity_permutation(...)

        /** @brief Modifies \p source by subtracting every element of \p other from it. */
        template <std::ranges::range t_source_container_type, std::ranges::range t_target_container_type>
        static bool try_subtract_assign(t_source_container_type& source, const t_target_container_type& other) noexcept
        {
            if (source.size() != other.size()) return false;
            auto it = other.begin();
            for (auto& x : source)
            {
                x -= (*it);
                ++it;
            } // for (...)
            return true;
        } // try_subtract_assign(...)

        /** @brief Modifies \p source by adding every element of \p other to it. */
        template <std::ranges::range t_source_container_type, std::ranges::range t_target_container_type>
        static bool try_add_assign(t_source_container_type& source, const t_target_container_type& other) noexcept
        {
            if (source.size() != other.size()) return false;
            auto it = other.begin();
            for (auto& x : source)
            {
                x += (*it);
                ++it;
            } // for (...)
            return true;
        } // try_add_assign(...)

        /** @brief Computes a binary mask for elements of \p source indicated by \p predicate.
         *  @param source The container of the elements to filter.
         *  @param predicate Predicate indicating which elements to select. Has to implement (const t_container_type::value_type&) -> bool.
         *  @remark \tparam t_integral_mask_type has to implement operator << and operator |.
         */
        template <std::ranges::range t_container_type, typename t_predicate_type, ropufu::binary_mask t_integral_mask_type>
            requires ropufu::one_assignable<t_integral_mask_type>
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
            requires ropufu::action_one<t_action_type, const typename t_container_type::value_type&>
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
            requires ropufu::action_one<t_action_type, typename t_container_type::value_type&>
        static void masked_action(t_container_type& source, t_integral_mask_type mask, t_action_type&& action) noexcept
        {
            for (auto& x : source)
            {
                if ((mask & 1) == 1) action(x);
                mask >>= 1;
            } // for (...)
        } // masked_action(...)
    }; // struct elementwise
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
