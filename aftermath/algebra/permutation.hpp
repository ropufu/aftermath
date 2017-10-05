
#ifndef AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED
#define AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED

#include "../not_an_error.hpp"

#include <algorithm>  // For std::sort.
#include <cstddef>
#include <functional> // For std::less.
#include <ostream>
#include <vector>

namespace ropufu
{
    namespace aftermath
    {
        namespace algebra
        {
            /** Describes a permutation. */
            struct permutation;

            /** An iterator for \c permutation to allow it to be used in range-based for loops. */
            class permutation_iterator
            {
                const permutation* m_permutation_pointer;
                std::size_t m_position;

            public:
                permutation_iterator(const permutation* p_permutation, std::size_t position) noexcept
                    : m_permutation_pointer(p_permutation), m_position(position)
                {
                }

                /** Termination condition. */
                bool operator !=(const permutation_iterator& other) const noexcept
                {
                    return this->m_position != other.m_position;
                }

                /** To be defined after \c permutation body. */
                std::size_t operator *() const noexcept;

                /** Increment operator. */
                permutation_iterator& operator ++() noexcept
                {
                    ++(this->m_position);
                    // Although not strictly necessary for a range-based for loop
                    // following the normal convention of returning a value from
                    // operator++ is a good idea.
                    return *this;
                }
            };

            /** @brief Describes a permutation.
             *  @remark This is a \c noexcept struct. Exception handling is done by \c quiet_error singleton.
             */
            struct permutation
            {
            private:
                std::vector<std::size_t> m_indices = {};

            public:
                /** An empty permutation. */
                permutation() noexcept
                {
                }

                /** An identity permutation with \p size many elements. */
                explicit permutation(std::size_t size) noexcept
                {
                    this->m_indices.reserve(size);
                    for (std::size_t i = 0; i < size; i++) this->m_indices.push_back(i);
                    this->m_indices.shrink_to_fit();
                }

                /** A permutation described by the elements in \p source. */
                template <typename t_source_type>
                explicit permutation(const std::vector<t_source_type>& source) noexcept
                    : permutation(source.size())
                {
                    this->order_by(source);
                }

                /** @brief Modify the permutation to reflect the ordering of elements in \p source.
                 *  @exception not_an_error::length_error This error is pushed to \c quiet_error if \p source is of different size than the permutation.
                 *  @remark \p source is intentionally passed as a copy.
                 */
                template <typename t_source_type, typename t_predicate_type>
                void order_by(std::vector<t_source_type> source, t_predicate_type comparer = std::less<t_source_type>()) noexcept
                {
                    if (this->m_indices.size() != source.size())
                    {
                        quiet_error::instance().push(not_an_error::length_error, severity_level::major, "Source size mismatch.", __FUNCTION__, __LINE__);
                        return;
                    }

                    std::sort(this->m_indices.begin(), this->m_indices.end(), 
                        [&source, &comparer] (std::size_t i, std::size_t j) {
                            return comparer(source[i], source[j]);
                    });
                }

                /** @brief Modify the permutation to reflect the ordering of elements in \p source.
                 *  @exception not_an_error::length_error This error is pushed to \c quiet_error if \p source is of different size than the permutation.
                 *  @remark \p source is intentionally passed as a copy.
                 */
                template <typename t_source_type, typename t_selector_type, typename t_predicate_type>
                void order_by(std::vector<t_source_type> source, t_selector_type selector, t_predicate_type comparer = std::less<t_source_type>()) noexcept
                {
                    if (this->m_indices.size() != source.size())
                    {
                        quiet_error::instance().push(not_an_error::length_error, severity_level::major, "Source size mismatch.", __FUNCTION__, __LINE__);
                        return;
                    }

                    std::sort(this->m_indices.begin(), this->m_indices.end(), 
                        [&source, &selector, &comparer] (std::size_t i, std::size_t j) {
                            return comparer(selector(source[i]), selector(source[j]));
                    });
                }

                /** Inverse of this permutation. */
                permutation invert() const noexcept
                {
                    permutation inverse;
                    std::size_t size = this->m_indices.size();
                    inverse.m_indices = std::vector<std::size_t>(size);
                    for (std::size_t i = 0; i < size; i++) inverse.m_indices[this->m_indices[i]] = i;
                    inverse.m_indices.shrink_to_fit();
                    return inverse;
                }

                /** Size of the permutation. */
                std::size_t size() const noexcept
                {
                    return this->m_indices.size();
                }

                /** @brief The destination of \p index as described by the permutation.
                 *  @remark No boundary checks are performed.
                 */
                std::size_t operator [](std::size_t index) const noexcept
                {
                    return this->m_indices[index];
                }

                /** @brief The destination of \p index as described by the permutation.
                 *  @exception not_an_error::out_of_range This error is pushed to \c quiet_error if \p index out of range.
                 */
                std::size_t at(std::size_t index) const noexcept
                {
                    if (index >= this->m_indices.size()) 
                    {
                        quiet_error::instance().push(not_an_error::out_of_range, severity_level::major, "<index> out of range.", __FUNCTION__, __LINE__);
                        return 0;
                    }
                    return this->operator [](index);
                }

                /** Iterator pointing to the beginning of the array. */
                permutation_iterator begin() const noexcept
                {
                    return permutation_iterator(this, 0);
                }

                /** Iterator pointing to the end of the array. */
                permutation_iterator end() const noexcept
                {
                    return permutation_iterator(this, this->size());
                }

                /** Outputs this permutation with comma-separated values to a stream. */
                friend std::ostream& operator <<(std::ostream& os, const permutation& that) noexcept
                {
                    bool is_first = true;
                    for (std::size_t index : that.m_indices)
                    {
                        if (is_first) is_first = false;
                        else os << ',';
                        os << index;
                    }
                    return os;
                }
            };

            /** Implementation of dereferencing for \c permutation_iterator. */
            std::size_t permutation_iterator::operator *() const noexcept
            {
                return this->m_permutation_pointer->operator [](this->m_position);
            }
        }
    }
}

#endif // AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED
