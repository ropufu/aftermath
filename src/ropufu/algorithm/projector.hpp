
#ifndef ROPUFU_AFTERMATH_ALGORITHM_PROJECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_PROJECTOR_HPP_INCLUDED

#include "../on_error.hpp"

#include "../algebra/matrix.hpp" // algebra::matrix
#include "../algebra/matrix_index.hpp" // algebra::matrix_index

#include <cstddef> // std::size_t
#include <type_traits> // std::is_same_v
#include <vector>  // std::vector

namespace ropufu::aftermath::algorithm
{
    template <typename t_size_type, typename t_cost_type>
    struct index_cost_pair
    {
        using size_type = t_size_type;
        using cost_type = t_cost_type;
        using index_type = algebra::matrix_index<size_type>;

        index_type index;
        cost_type cost;

        index_cost_pair() noexcept = default;

        index_cost_pair(const index_type& index, const cost_type& cost) noexcept
            : index(index), cost(cost)
        {
        } // index_cost_pair(...)
    }; // struct index_cost_pair

    template <typename t_derived_type, typename t_surface_type, typename t_cost_type>
    struct projector;

    template <typename t_projector_type>
    using projector_t = projector<t_projector_type,
        typename t_projector_type::surface_type,
        typename t_projector_type::cost_type>;

    /** @brief Projects a surface onto a two-dimensional matrix. */
    template <typename t_derived_type, typename t_surface_type, typename t_cost_type>
    struct projector
    {
        using type = projector<t_derived_type, t_surface_type, t_cost_type>;
        using derived_type = t_derived_type;

        using surface_type = t_surface_type;
        using cost_type = t_cost_type;

        using index_type = algebra::matrix_index<std::size_t>;
        using pair_type = index_cost_pair<std::size_t, cost_type>;

    private:
        surface_type m_surface = {}; // Surface to walk on.

        static constexpr void traits_check()
        {
        } // traits_check(...)

    protected:
        std::size_t height_override() const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::height_override), 
                decltype(&type::height_override)>;
            static_assert(is_overwritten, "height_override() const noexcept -> std::size_t was not overwritten.");

            const derived_type* that = static_cast<const derived_type*>(this);
            return that->height_override();
        } // height_override(...)

        std::size_t width_override() const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::width_override), 
                decltype(&type::width_override)>;
            static_assert(is_overwritten, "width_override() const noexcept -> std::size_t was not overwritten.");

            const derived_type* that = static_cast<const derived_type*>(this);
            return that->width_override();
        } // width_override(...)

        cost_type distance_override(const index_type& a, const index_type& b) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::distance_override), 
                decltype(&type::distance_override)>;
            static_assert(is_overwritten, "distance_override(const index_type&, const index_type&) const noexcept -> cost_type was not overwritten.");

            const derived_type* that = static_cast<const derived_type*>(this);
            return that->distance_override(a, b);
        } // distance_override(...)

        void neighbors_override(const index_type& source, std::vector<pair_type>& projected_neighbors) const noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::neighbors_override), 
                decltype(&type::neighbors_override)>;
            static_assert(is_overwritten, "neighbors_override(const index_type&, std::vector<pair_type>&) const noexcept -> void was not overwritten.");

            const derived_type* that = static_cast<const derived_type*>(this);
            that->neighbors_override(source, projected_neighbors);
        } // neighbors_override(...)

    public:
        projector() noexcept
        {
            type::traits_check();
        } // projector(...)

        explicit projector(const surface_type& surface) noexcept
            : m_surface(surface)
        {
            type::traits_check();
        } // projector(...)

        const surface_type& surface() const noexcept { return this->m_surface; }
        
        std::size_t height() const noexcept { return this->height_override(); }
        std::size_t width() const noexcept { return this->width_override(); }

        cost_type distance(const index_type& a, const index_type& b) const noexcept { return this->distance_override(a, b); }

        void neighbors(const index_type& source, std::vector<pair_type>& projected_neighbors) const noexcept
        {
            this->neighbors_override(source, projected_neighbors);
        } // neighbors(...)
    }; // struct walker

    namespace detail
    {
        template <typename t_value_type>
        struct inequality_comparer
        {
            static constexpr bool good(const t_value_type& value, const t_value_type& blocked_indicator) noexcept
            {
                return value != blocked_indicator;
            } // good(...)
        }; // struct inequality_comparer

        template <>
        struct inequality_comparer<bool>
        {
            static constexpr bool good(const bool& value, const bool& blocked_indicator) noexcept
            {
                return !(value ^ blocked_indicator);
            } // good(...)
        }; // struct inequality_comparer
    } // namespace detail

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    struct matrix_projector;

    template <typename t_matrix_type>
    using matrix_projector_t = matrix_projector<typename t_matrix_type::value_type,
        typename t_matrix_type::arrangement_type,
        typename t_matrix_type::allocator_type>;

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    struct matrix_projector : public projector<
        matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>,
        algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>,
        typename algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>::size_type>
    {
        using type = matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>;
        using surface_type = algebra::matrix<bool, t_arrangement_type, t_allocator_type>;
        using cost_type = typename surface_type::size_type;

        using base_type = projector<type, surface_type, cost_type>;
        using value_type = t_value_type;
        using cell_comparer_type = detail::inequality_comparer<value_type>;

        using index_type = algebra::matrix_index<std::size_t>;
        using pair_type = index_cost_pair<std::size_t, cost_type>;

        friend base_type;

    private:
        value_type m_blocked_indicator = {}; // Value indicating that the cell is blocked.

    protected:
        std::size_t height_override() const noexcept 
        {
            return this->surface().height();
        } // height_override(...)

        std::size_t width_override() const noexcept 
        {
            return this->surface().width();
        } // width_override(...)

        /** L1 distance between two indices. */
        cost_type distance_override(const index_type& a, const index_type& b) const noexcept
        {
            cost_type dx = (a.column < b.column) ? (b.column - a.column) : (a.column - b.column);
            cost_type dy = (a.row < b.row) ? (b.row - a.row) : (a.row - b.row);
            return dx + dy;
        } // distance_override(...)

        void neighbors_override(const index_type& source, std::vector<pair_type>& projected_neighbors) const noexcept
        {
            const surface_type& surface = this->surface();

            projected_neighbors.clear();
            projected_neighbors.reserve(4);

            if (source.row != 0)
            {
                index_type neighbor_index = source;
                --neighbor_index.row;
                if (cell_comparer_type::good(surface[neighbor_index], this->m_blocked_indicator))
                    projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.column != this->width() - 1)
            {
                index_type neighbor_index = source;
                ++neighbor_index.column;
                if (cell_comparer_type::good(surface[neighbor_index], this->m_blocked_indicator))
                    projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.row != this->height() - 1)
            {
                index_type neighbor_index = source;
                ++neighbor_index.row;
                if (cell_comparer_type::good(surface[neighbor_index], this->m_blocked_indicator))
                    projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
            if (source.column != 0)
            {
                index_type neighbor_index = source;
                --neighbor_index.column;
                if (cell_comparer_type::good(surface[neighbor_index], this->m_blocked_indicator))
                    projected_neighbors.emplace_back(neighbor_index, 1);
            } // if (...)
        } // neighbors_override(...)

    public:
        using base_type::projector; // Inherit constructors.

        matrix_projector(const surface_type& surface, const value_type& blocked_indicator) noexcept
            : base_type(surface), m_blocked_indicator(blocked_indicator)
        {
        } // matrix_projector(...)
    }; // struct matrix_projector

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    static auto make_matrix_projector(
        const algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>& surface,
        const t_value_type& blocked_indicator = {}) noexcept
        -> matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>
    {
        return {surface, blocked_indicator};
    } // make_matrix_projector(...)
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_PROJECTOR_HPP_INCLUDED
