
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ELEMENTWISE_HPP_INCLUDED

#include "../not_an_error.hpp"

#include <cstddef>
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
				 *  @remark \p t_container_type has to implement \c size() and indexing operator [\c std::size_t] -> \p t_value_type.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the sizes of arguments do not match.
				 */
				template <typename t_container_type, typename t_value_type = typename t_container_type::value_type>
				static void subtract_assign(t_container_type& source, const t_container_type&& other) noexcept
				{
                    if (source.size() != other.size())
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::fatal, "Sizes should match.", __FUNCTION__, __LINE__);
                        return;
                    }
					for (std::size_t i = 0; i < source.size(); i++) source[i] -= other[i];
				}

				/** @brief Modifies \p source by adding every element of \p other to it.
				 *  @param source The container to add to.
				 *  @param other The container to be added.
				 *  @remark \p t_container_type has to define typename \c value_type, the type of its elements.
				 *  @remark \p t_container_type has to implement \c size() and indexing operator [\c std::size_t] -> \p t_value_type.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the sizes of arguments do not match.
				 */
				template <typename t_container_type, typename t_value_type = typename t_container_type::value_type>
				static void add_assign(t_container_type& source, const t_container_type&& other) noexcept
				{
                    if (source.size() != other.size())
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::fatal, "Sizes should match.", __FUNCTION__, __LINE__);
                        return;
                    }
					for (std::size_t i = 0; i < source.size(); i++) source[i] += other[i];
				}

				/** @brief Adds up elements of \p source marked by \p binary_mask.
				 *  @param source The container of the elements to sum.
				 *  @param binary_mask Binary mask indicating which elements to sum.
				 *  @remark \p t_container_type has to define typename \c value_type, the type of its elements.
				 *  @remark \p t_container_type has to implement \c size() and indexing operator [\c std::size_t] -> \p t_value_type.
				 *  @remark \p t_mask_type has to implement right shift operator >>.
           		 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p binary_mask exceeds the size of \p source.
				 */
				template <typename t_container_type, typename t_mask_type, typename t_value_type = typename t_container_type::value_type>
				static t_value_type sum_binary_mask(const t_container_type& source, t_mask_type binary_mask) noexcept
				{
					typename t_container_type::value_type sum = 0;

					std::size_t index = 0;
					std::size_t count = source.size();
					while (binary_mask > 0)
					{
						if (index >= count)
						{
							quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Binary mask invalid.", __FUNCTION__, __LINE__);
							return 0;
						}

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
           		 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p binary_mask exceeds the size of \p source.
				 */
				template <typename t_value_type, typename t_mask_type>
				static std::vector<t_value_type> from_binary_mask(const std::vector<t_value_type>& source, t_mask_type binary_mask) noexcept
				{
					std::vector<t_value_type> result = { };
					std::size_t index = 0;
					std::size_t count = source.size();
					result.reserve(count);

					while (binary_mask > 0)
					{
						if (index >= count)
						{
							quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "Binary mask invalid.", __FUNCTION__, __LINE__);
							return { };
						}

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
				 *  @remark \p t_mask_container_type has to implement \c size() and indexing operator [\c std::size_t] -> \p bool.
                 *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if the sizes of arguments do not match.
				 */
				template <typename t_value_type, typename t_mask_container_type>
				static std::vector<t_value_type> from_boolean_mask(const std::vector<t_value_type>& source, const t_mask_container_type& boolean_mask) noexcept
				{
					std::vector<t_value_type> result = { };
					std::size_t count = source.size();
					result.reserve(count);

                    if (count != boolean_mask.size())
                    {
                        quiet_error::instance().push(not_an_error::logic_error, severity_level::major, "Sizes should match.", __FUNCTION__, __LINE__);
                        return { };
                    }
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
