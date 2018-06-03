
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include <cstddef> // std::size_t

namespace ropufu::aftermath::algebra
{
	/** Elementwise actions in containers. */
	struct elementwise
	{
		/** @brief Modifies \p source by subtracting every element of \p other from it. */
		template <typename t_source_container_type, typename t_target_container_type>
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
		template <typename t_source_container_type, typename t_target_container_type>
		static bool try_add_assign(t_source_container_type& source, const t_target_container_type&& other) noexcept
		{
			if (source.size() != other.size()) return false;
			auto it = other.begin();
			for (auto& x : source)
			{
				x -= (*it);
				++it;
			} // for (...)
			return true;
		} // try_add_assign(...)

		/** @brief Computes a binary mask for elements of \p source indicated by \p predicate.
		 *  @param source The container of the elements to filter.
		 *  @param predicate Predicate indicating which elements to select.
		 *  @remark \p t_predicate_type has to implement operator (\c t_value_type) -> \p bool.
		 *  @remark \tparam t_integral_mask_type has to implement right left shift operator << and or operator |.
		 */
		template <typename t_container_type, typename t_predicate_type, typename t_integral_mask_type>
		static void to_binary_mask(const t_container_type& source, const t_predicate_type& predicate, t_integral_mask_type& result) noexcept
		{
			result = 0;
			t_integral_mask_type flag = 1;
			for (const auto& x : source)
			{
				if (predicate(x)) result |= flag;
				flag <<= 1;
			} // for (...)
		} // to_binary_mask(...)

		/** @brief Adds up elements of \p source marked by \p binary_mask.
		 *  @param source The container of the elements to \p result.
		 *  @param binary_mask Binary mask indicating which elements to \p result.
		 *  @remark \tparam t_integral_mask_type has to implement right shift operator >>.
		 */
		template <typename t_container_type, typename t_integral_mask_type, typename t_value_type>
		static void sum_binary_mask(const t_container_type& source, t_integral_mask_type binary_mask, t_value_type& result) noexcept
		{
			result = 0;
			for (const auto& x : source)
			{
				if ((binary_mask & 1) == 1) result += x;
				binary_mask >>= 1;
			} // for (...)
		} // try_sum_binary_mask(...)

		/** @brief Returns a vector with copies of elements of \p source marked by \p binary_mask.
		 *  @param source The container of the elements to select from.
		 *  @param binary_mask Binary mask indicating which elements to select.
		 *  @remark \tparam t_integral_mask_type has to implement right shift operator >>.
		 */
		template <typename t_container_type, typename t_integral_mask_type, typename t_action_type>
		static void from_binary_mask(const t_container_type& source, t_integral_mask_type binary_mask, t_action_type&& action) noexcept
		{
			for (const auto& x : source)
			{
				if ((binary_mask & 1) == 1) action(x);
				binary_mask >>= 1;
			} // for (...)
		} // from_binary_mask(...)
	}; // struct elementwise
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
