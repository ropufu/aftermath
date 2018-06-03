
#ifndef ROPUFU_AFTERMATH_FORMAT_GRID_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_GRID_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <type_traits> // ...

namespace ropufu::aftermath::format
{
    /** @brief Distributed computation manager. */
    template <typename t_derived_type = void>
    struct grid
    {
        using type = grid<t_derived_type>;
        using derived_type = t_derived_type;

        /** Indicates if the current machine is the master node. */
        bool is_master() const noexcept 
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::is_master), 
                decltype(&type::is_master)>;
            static_assert(is_overwritten, "static polymorphic function <is_master> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->is_master();
        } // is_master(...)

        /** Synchronization point for all machines in the grid. */
        void barrier() noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::barrier), 
                decltype(&type::barrier)>;
            static_assert(is_overwritten, "static polymorphic function <barrier> was not overwritten.");
            derived_type* that = static_cast<derived_type*>(this);
            return that->barrier();
        } // barrier(...)

        /** Translates local row index to global row index. */
        std::size_t row_local_to_global(std::size_t local_row_index) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::row_local_to_global), 
                decltype(&type::row_local_to_global)>;
            static_assert(is_overwritten, "static polymorphic function <row_local_to_global> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->row_local_to_global(local_row_index);
        } // row_local_to_global(...)

        /** Translates local column index to global column index. */
        std::size_t column_local_to_global(std::size_t local_column_index) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::column_local_to_global), 
                decltype(&type::column_local_to_global)>;
            static_assert(is_overwritten, "static polymorphic function <column_local_to_global> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->column_local_to_global(local_column_index);
        } // column_local_to_global(...)

        /** Translates global row index to local row index. */
        std::size_t row_global_to_local(std::size_t global_row_index) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::row_global_to_local), 
                decltype(&type::row_global_to_local)>;
            static_assert(is_overwritten, "static polymorphic function <row_global_to_local> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->row_global_to_local(global_row_index);
        } // row_global_to_local(...)

        /** Translates global column index to local column index. */
        std::size_t column_global_to_local(std::size_t global_column_index) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::column_global_to_local), 
                decltype(&type::column_global_to_local)>;
            static_assert(is_overwritten, "static polymorphic function <column_global_to_local> was not overwritten.");
            const derived_type* that = static_cast<const derived_type*>(this);
            return that->column_global_to_local(global_column_index);
        } // column_global_to_local(...)
    }; // struct grid

    /** A trivial grid representing a single-machine scenario. */
    template <>
    struct grid<void>
    {
        /** Indicates if the current machine is the master node. */
        constexpr bool is_master() const noexcept { return true; }

        /** Synchronization point for all machines in the grid. */
        constexpr void barrier() const noexcept { }

        /** Translates local row index to global row index. */
        std::size_t row_local_to_global(std::size_t local_row_index) const noexcept { return local_row_index; }

        /** Translates local column index to global column index. */
        std::size_t column_local_to_global(std::size_t local_column_index) const noexcept { return local_column_index; }

        /** Translates global row index to local row index. */
        std::size_t row_global_to_local(std::size_t global_row_index) const noexcept { return global_row_index; }

        /** Translates global column index to local column index. */
        std::size_t column_global_to_local(std::size_t global_column_index) const noexcept { return global_column_index; }
    }; // struct grid<...>
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_GRID_HPP_INCLUDED
