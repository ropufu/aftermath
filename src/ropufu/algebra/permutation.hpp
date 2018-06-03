
#ifndef AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED
#define AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED

#include <algorithm>  // std::sort
#include <cstddef>    // std::size_t, std::nullptr_t
#include <functional> // std::less
#include <ostream>    // std::ostream
#include <utility>    // std::declval
#include <vector>     // std::vector

namespace ropufu::aftermath::algebra
{
    /** @brief Describes a permutation. */
    template <typename t_size_type = std::size_t>
    struct permutation
    {
        using type = permutation<t_size_type>;
        using size_type = t_size_type;

    private:
        std::vector<size_type> m_indices = {};

        /** An identity permutation with \p size many elements. */
        permutation(std::nullptr_t, const type& to_invert) noexcept
            : m_indices(to_invert.m_indices.size())
        {
            std::size_t size = this->m_indices.size();
            for (size_type i = 0; i < size; ++i) this->m_indices[to_invert.m_indices[static_cast<std::size_t>(i)]] = i;
            this->m_indices.shrink_to_fit();
        } // permutation(...)

    public:
        /** An empty permutation. */
        permutation() noexcept { }

        /** An identity permutation with \p size many elements. */
        explicit permutation(size_type size) noexcept
            : m_indices(static_cast<std::size_t>(size))
        {
            for (size_type i = 0; i < size; ++i) this->m_indices[static_cast<std::size_t>(i)] = i;
            this->m_indices.shrink_to_fit();
        } // permutation(...)

        /** A permutation described by the elements in \p source. */
        template <typename t_container_type>
        explicit permutation(const t_container_type& source) noexcept
            : permutation(static_cast<size_type>(source.size()))
        {
            this->try_order_by(source);
        } // permutation(...)

        /** @brief Modify the permutation to reflect the ordering of elements in \p source.
         *  @remark \tparam t_container_type has to define type \c value_type and implement \c operator [](std::size_t) -> value_type.
         */
        template <typename t_container_type, typename t_comparer_type>
        bool try_order_by(const t_container_type& source, const t_comparer_type& comparer = std::less<typename t_container_type::value_type>()) noexcept
        {
            if (this->m_indices.size() != source.size()) return false; // Size mismatch.

            std::sort(this->m_indices.begin(), this->m_indices.end(), 
                [&source, &comparer] (size_type i, size_type j) {
                    return comparer(source[static_cast<std::size_t>(i)], source[static_cast<std::size_t>(j)]);
            }); // std::sort(...)
            return true;
        } // try_order_by(...)

        /** @brief Modify the permutation to reflect the ordering of elements in \p source.
         *  @remark \tparam t_container_type has to define type \c value_type and implement \c operator [](std::size_t) -> ...
         *  @remark \tparam t_selector_type has to implement \c operator ()(...) -> value_type.
         */
        template <typename t_container_type, typename t_selector_type, typename t_comparer_type>
        bool try_order_by(const t_container_type& source,
            t_selector_type&& selector, 
            const t_comparer_type& comparer = std::less< decltype(selector(std::declval<const typename t_container_type::value_type&>())) >()) noexcept
        {
            if (this->m_indices.size() != source.size()) return false; // Size mismatch.

            std::sort(this->m_indices.begin(), this->m_indices.end(), 
                [&source, &selector, &comparer] (size_type i, size_type j) {
                    return comparer(selector(source[static_cast<std::size_t>(i)]), selector(source[static_cast<std::size_t>(j)]));
            }); // std::sort(...)
            return true;
        } // try_order_by(...)

        /** Inverse of this permutation. */
        type inverse() const noexcept { return type(nullptr, this); }

        /** Size of the permutation. */
        std::size_t size() const noexcept { return this->m_indices.size(); }

        /** @brief The destination of \p index as described by the permutation. */
        size_type operator [](std::size_t index) const { return this->m_indices[index]; }

        /** @brief The destination of \p index as described by the permutation. */
        std::size_t at(std::size_t index) const { return this->m_indices.at(index); }

        auto cbegin() const noexcept { return this->m_indices.cbegin(); }
        auto cend() const noexcept { return this->m_indices.cend(); }

        auto begin() const noexcept { return this->m_indices.begin(); }
        auto end() const noexcept { return this->m_indices.end(); }

        auto begin() noexcept { return this->m_collection.begin(); }
        auto end() noexcept { return this->m_collection.end(); }

        /** Outputs this permutation with comma-separated values to a stream. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            bool is_first = true;
            for (size_type index : self.m_indices)
            {
                if (is_first) is_first = false;
                else os << ',';
                os << index;
            } // for (...)
            return os;
        } // operator <<(...)
    }; // struct permutation
} // namespace ropufu::aftermath::algebra

#endif // AFTERMATH_ALGEBRA_PERMUTATION_HPP_INCLUDED
