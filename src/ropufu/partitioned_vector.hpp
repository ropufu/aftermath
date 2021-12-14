
#ifndef ROPUFU_AFTERMATH_PARTITIONED_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PARTITIONED_VECTOR_HPP_INCLUDED

#include "simple_vector.hpp"
#include "vector_extender.hpp"

#include <concepts>  // std::default_initializable, std::same_as
#include <initializer_list> // std::initializer_list
#include <memory>    // std::allocator, std::allocator_traits
#include <ranges>    // std::ranges
#include <stdexcept> // std::out_of_range, std::logic_error
#include <vector>    // std::vector

namespace ropufu::aftermath
{
    template <std::default_initializable t_value_type, typename t_allocator_type = std::allocator<t_value_type>>
    struct partitioned_vector;

    template <std::default_initializable t_value_type, typename t_allocator_type>
    struct partitioned_vector
    {
        using type = partitioned_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;
        using vector_type = aftermath::simple_vector<value_type, allocator_type>;

        using allocator_traits_type = std::allocator_traits<allocator_type>;
        using size_type = typename allocator_traits_type::size_type;
        using iterator_type = value_type*;
        using const_iterator_type = const value_type*;

    private:
        /** Underlying (unoptimized) storage. */
        vector_type m_values = {};
        /** @example [0, 1, 5, 7, 7] defines a partition with 4 elements; last element is trivial.  */
        std::vector<size_type> m_border_indices = std::vector<std::size_t>(2);
        /** Indicates which parition is active to facilitate iteration. */
        size_type m_active_partition_index = 0;

    public:
        partitioned_vector() noexcept
        {
        } // partitioned_vector(...)

        /** Partition spanning the entire vector. */
        explicit partitioned_vector(std::initializer_list<value_type> container)
            : m_values(container)
        {
            this->m_border_indices[1] = this->m_values.size();
        } // simple_vector(...)

        /** Partition spanning the entire vector. */
        template <std::ranges::range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        explicit partitioned_vector(const t_container_type& container)
            : m_values(container)
        {
            this->m_border_indices[1] = this->m_values.size();
        } // partitioned_vector(...)

        /** Number of sub-blocks in the vector. */
        std::size_t partition_size() const noexcept { return this->m_border_indices.size() - 1; }

        /** Marks a partition as the one to iterate through. */
        void activate_partition(size_type partition_index)
        {
            if (partition_index >= this->partition_size()) throw std::out_of_range("Partition index out of range.");
            this->m_active_partition_index = partition_index;
        } // activate_partition(...)

        /** Refines the partition by splitting an existing element at the specified location. */
        void split(size_type border_index)
        {
            if (border_index > this->m_values.size()) throw std::logic_error("Sentinel index cannot exceed vector size.");

            this->m_border_indices.push_back(border_index);
            ropufu::sort(this->m_border_indices);
        } // split(...)

        void clear_partition() noexcept
        {
            this->m_border_indices.clear();
            this->m_border_indices.emplace_back();
            this->m_border_indices.emplace_back(this->m_values.size());
            this->m_active_partition_index = 0;
        } // clear_partition(...)

        const_iterator_type cbegin() const noexcept { return this->m_values.cbegin() + this->m_border_indices[this->m_active_partition_index]; }
        const_iterator_type cend() const noexcept { return this->m_values.cbegin() + this->m_border_indices[this->m_active_partition_index + 1]; }

        const_iterator_type begin() const noexcept { return this->m_values.begin() + this->m_border_indices[this->m_active_partition_index]; }
        const_iterator_type end() const noexcept { return this->m_values.begin() + this->m_border_indices[this->m_active_partition_index + 1]; }

        iterator_type begin() noexcept { return this->m_values.begin() + this->m_border_indices[this->m_active_partition_index]; }
        iterator_type end() noexcept { return this->m_values.begin() + this->m_border_indices[this->m_active_partition_index + 1]; }
    }; // struct partitioned_vector
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_PARTITIONED_VECTOR_HPP_INCLUDED
