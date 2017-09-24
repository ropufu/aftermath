
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace ropufu
{
	namespace aftermath
	{
		namespace algebra
		{
			/** Elementwise actions in containers. */
			struct elementwise
			{
				/** @brief Modifies \p source by subtracting every element of \p other from it.
				 *  @param source The container to subtract from.
				 *  @param other The container to be subtracted.
				 *  @remark \p t_container_type has to define typename \c value_type, the type of its elements.
				 *  @remark \p t_container_type has to implement size() and indexing operator [\c std::size_t] -> \p t_value_type.
				 */
				template <typename t_container_type, typename t_value_type = typename t_container_type::value_type>
				static void subtract_assign(t_container_type& source, const t_container_type&& other) noexcept
				{
					for (std::size_t i = 0; i < source.size(); i++) source[i] -= other[i];
				}

				/** @brief Modifies \p source by adding every element of \p other to it.
				 *  @param source The container to add to.
				 *  @param other The container to be added.
				 *  @remark \p t_container_type has to define typename \c value_type, the type of its elements.
				 *  @remark \p t_container_type has to implement size() and indexing operator [\c std::size_t] -> \p t_value_type.
				 */
				template <typename t_container_type, typename t_value_type = typename t_container_type::value_type>
				static void add_assign(t_container_type& source, const t_container_type&& other) noexcept
				{
					for (std::size_t i = 0; i < source.size(); i++) source[i] += other[i];
				}

				/** @brief Adds up elements of \p source marked by \p binary_mask.
				 *  @param source The container of the elements to sum.
				 *  @param binary_mask Binary mask indicating which elements to sum.
				 *  @remark \p t_container_type has to define typename \c value_type, the type of its elements.
				 *  @remark \p t_container_type has to implement size() and indexing operator [\c std::size_t] -> \p t_value_type.
				 *  @remark \p t_mask_type has to implement right shift operator >>.
           		 *  @exception std::out_of_range \p binary_mask is exceeds the size of \p source.
				 */
				template <typename t_container_type, typename t_mask_type, typename t_value_type = typename t_container_type::value_type>
				static t_value_type sum_binary_mask(const t_container_type& source, t_mask_type binary_mask)
				{
					typename t_container_type::value_type sum = 0;

					std::size_t index = 0;
					std::size_t count = source.size();
					while (binary_mask > 0)
					{
						if (index >= count) throw std::out_of_range("<binary_mask> invalid.");

						if ((binary_mask & 1) == 1) sum += source[index];
						binary_mask >>= 1;
						index++;
					}
					return sum;
				}

				/** @brief Returns a vector with copies of elements of \p source marked by \p binary_mask.
				 *  @param source The container of the elements to select from.
				 *  @param binary_mask Binary mask indicating which elements to select.
				 *  @remark \p t_mask_type has to implement right shift operator >>.
           		 *  @exception std::out_of_range \p binary_mask is exceeds the size of \p source.
				 */
				template <typename t_value_type, typename t_mask_type>
				static std::vector<t_value_type> from_binary_mask(const std::vector<t_value_type>& source, t_mask_type binary_mask)
				{
					std::vector<t_value_type> result = {};
					std::size_t index = 0;
					std::size_t count = source.size();
					result.reserve(count);

					while (binary_mask > 0)
					{
						if (index >= count) throw std::out_of_range("<binary_mask> invalid.");

						if ((binary_mask & 1) == 1) result.push_back(source[index]);
						binary_mask >>= 1;
						index++;
					}
					result.shrink_to_fit();
					return result;
				}
				
				/** @brief Returns a vector with copies of elements of \p source marked by \p boolean_mask.
				 *  @param source The container of the elements to select from.
				 *  @param boolean_mask Boolean mask indicating which elements to select.
				 *  @remark \p t_mask_container_type has to implement indexing operator [\c std::size_t] -> \p bool.
				 *  @exception std::out_of_range \p binary_mask is exceeds the size of \p source.
				 */
				template <typename t_value_type, typename t_mask_container_type>
				static std::vector<t_value_type> from_boolean_mask(const std::vector<t_value_type>& source, const t_mask_container_type& boolean_mask) noexcept
				{
					std::vector<t_value_type> result = {};
					std::size_t count = source.size();
					result.reserve(count);

					for (std::size_t index = 0; index < count; index++) if (boolean_mask[index]) result.push_back(source[index]);

					result.shrink_to_fit();
					return result;
				}

				/** @brief Computes a binary mask for elements of \p source indicated by \p predicate.
				 *  @param source The container of the elements to filter.
				 *  @param predicate Predicate indicating which elements to select.
				 *  @remark \p t_predicate_type has to implement operator (\c t_value_type) -> \p bool.
				 */
				template <typename t_value_type, typename t_predicate_type, typename t_mask_type = std::size_t>
				static t_mask_type to_binary_mask(const std::vector<t_value_type>& source, const t_predicate_type& predicate) noexcept
				{
					t_mask_type flag = 1;
					t_mask_type mask = 0;

					for (const t_value_type& x : source)
					{
						if (predicate(x)) mask |= flag;
						flag <<= 1;
					}
					return mask;
				}
			};
		}
	}
}

#endif // ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
