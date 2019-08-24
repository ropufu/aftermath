
#ifndef ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED

#include "algebra/matrix.hpp"

#include "algorithm/fuzzy.hpp"
#include "algorithm/pathfinder.hpp"
#include "algorithm/projector.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    template <typename t_argument_type, typename t_value_type>
    using fuzzy_t = aftermath::algorithm::fuzzy<t_argument_type, t_value_type>;

    template <typename t_size_type, typename t_cost_type>
    using index_cost_pair_t = aftermath::algorithm::index_cost_pair<t_size_type, t_cost_type>;

    template <typename t_projector_type>
    using projector_t = aftermath::algorithm::projector_t<t_projector_type>;

    template <typename t_matrix_type>
    using matrix_projector_t = aftermath::algorithm::matrix_projector_t<t_matrix_type>;

    template <typename t_value_type, typename t_arrangement_type, typename t_allocator_type>
    static auto make_matrix_projector(
        const aftermath::algebra::matrix<t_value_type, t_arrangement_type, t_allocator_type>& surface,
        const t_value_type& blocked_indicator = {}) noexcept
        -> aftermath::algorithm::matrix_projector<t_value_type, t_arrangement_type, t_allocator_type>
    {
        return aftermath::algorithm::make_matrix_projector(surface, blocked_indicator);
    } // make_matrix_projector(...)

    template <typename t_projector_type>
    using pathfinder_t = aftermath::algorithm::pathfinder<t_projector_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_ALGORITHM_HPP_INCLUDED
