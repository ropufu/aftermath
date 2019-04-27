
#ifndef ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED

#include "algebra/matrix.hpp"

#include "algorithm/projector.hpp"
#include "algorithm/pathfinder.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_size_type, typename t_cost_type>
    using index_cost_pair = ropufu::aftermath::algorithm::index_cost_pair<t_size_type, t_cost_type>;

    template <typename t_derived_type, typename t_surface_type, typename t_cost_type>
    using projector = ropufu::aftermath::algorithm::projector<t_derived_type, t_surface_type, t_cost_type>;

    template <typename t_projector_type>
    using projector_t = projector<t_projector_type,
        typename t_projector_type::surface_type,
        typename t_projector_type::cost_type>;

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    using matrix_projector = ropufu::aftermath::algorithm::matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>;

    template <typename t_matrix_type>
    using matrix_projector_t = matrix_projector<typename t_matrix_type::value_type,
        typename t_matrix_type::arrangement_type,
        typename t_matrix_type::allocator_type>;

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    static auto make_matrix_projector(
        const aftermath::algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>& surface,
        const t_value_type& blocked_indicator = {}) noexcept
        -> matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>
    {
        return {surface, blocked_indicator};
    } // make_matrix_projector(...)

    template <typename t_projector_type>
    using pathfinder = ropufu::aftermath::algorithm::pathfinder<t_projector_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED
