
#ifndef ROPUFU_AFTERMATH_ALGORITHM_PATHFINDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_PATHFINDER_HPP_INCLUDED

#include "../on_error.hpp"

#include "../algebra/matrix.hpp" // algebra::matrix
#include "../algebra/matrix_index.hpp" // algebra::matrix_index

#include "projector.hpp"

#include <cstddef>      // std::size_t
#include <system_error> // std::error_code, std::errc
#include <type_traits>  // std::is_base_of_v
#include <map>          // std::multimap
#include <system_error> // std::error_code, std::errc
#include <vector>       // std::vector

namespace ropufu::aftermath::algorithm
{
    namespace detail
    {
        template <typename t_size_type, typename t_cost_type>
        struct pathfinder_node
        {
            using size_type = t_size_type;
            using cost_type = t_cost_type;
            using index_type = algebra::matrix_index<size_type>;

            cost_type cost_from_source; // "g score": the cost of getting here from the start node.
            bool open;   // Indicates that this node is part of the "open set": its \c cost_from_source has been recorded, but neighbors have not been processed.
            bool closed; // Indicates that this node is part of the "closed set": its neighbors have been recorded.
            index_type came_from; // Index of the neighbor node this one can be most efficiently reached from.
        }; // struct pathfinder_node
    } // namespace detail

    template <typename t_projector_type>
    struct pathfinder;

    template <typename t_projector_type>
    static void trace(const algebra::matrix_index<std::size_t>& from, const algebra::matrix_index<std::size_t>& to,
        const t_projector_type& projector,
        std::vector<algebra::matrix_index<std::size_t>>& path, std::error_code& ec) noexcept
    {
        pathfinder<t_projector_type> router {projector, from, ec};
        if (ec.value() != 0) return;
        router.trace(to, path, ec);
    } // trace(...)

    /** @brief Traces a shortest path on a surface from one index to another. Inspired by the A-star algorithm.
     *  @reference https://en.wikipedia.org/wiki/A*_search_algorithm.
     */
    template <typename t_projector_type>
    struct pathfinder
    {
        using type = pathfinder<t_projector_type>;
        using projector_type = t_projector_type;
        using surface_type = typename projector_type::surface_type;
        using cost_type = typename projector_type::cost_type;

        using index_type = algebra::matrix_index<std::size_t>;
        using node_type = detail::pathfinder_node<std::size_t, cost_type>;
        using pair_type = index_cost_pair<std::size_t, cost_type>;

        static constexpr std::size_t default_neighbor_capacity = 4;

    private:
        projector_type m_projector = {};
        index_type m_source = {}; // The starting point of the path.
        algebra::matrix<node_type> m_traceback = {}; // Information about the nodes allowing to optimally travel from \c m_source.
        std::multimap<cost_type, index_type> m_pending = {}; // The set of currently discovered nodes that are not completely evaluated yet, a.k.a. "open set".
        std::vector<pair_type> m_temp_neighbors = {};

        static constexpr void traits_check()
        {
            static_assert(std::is_base_of_v<projector_t<projector_type>, projector_type>,
                "projector_type has to directly derive from projector<projector_type, ...>.");
        } // traits_check(...)

        bool validate(std::error_code& ec) const noexcept
        {
            if (this->m_source.row < 0 || this->m_source.row >= this->m_traceback.height()) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Source must be within surface boundary.", false);
            if (this->m_source.column < 0 || this->m_source.column >= this->m_traceback.width()) return aftermath::detail::on_error(ec, std::errc::invalid_argument, "Source must be within surface boundary.", false);
            return true;
        } // validate(...)

        void coerce() noexcept
        {
            if (this->m_source.row < 0 || this->m_source.row >= this->m_traceback.height()) this->m_source.row = 0;
            if (this->m_source.column < 0 || this->m_source.column >= this->m_traceback.width()) this->m_source.column = 0;
        } // coerce(...)

        void enqueue(const index_type& position, const index_type& came_from, const index_type& target, cost_type cost_from_source) noexcept
        {
            if (this->m_traceback[position].closed) return; // Skip nodes that have already been processed.
            if (this->m_traceback[position].open) return; // Skip nodes that have already been marked for processing.

            const projector_t<projector_type>& projector_ref = this->m_projector;
            std::size_t estimated_cost_to_target = cost_from_source + projector_ref.distance(position, target);

            this->m_traceback[position].cost_from_source = cost_from_source; // Each pending node has to have a corresponding entry in the cost matrix.
            this->m_traceback[position].open = true;
            this->m_traceback[position].came_from = came_from;
            this->m_pending.emplace(estimated_cost_to_target, position);
        } // enqueue(...)

        /** Expands \c m_pending by processing the cheapest extimated nodes and enqueueing its neighbors. */
        void expand(const index_type& target) noexcept
        {
            if (this->m_pending.empty()) return; // We've exhausted all nodes!
            const projector_t<projector_type>& projector_ref = this->m_projector;

            auto cheepest_it = this->m_pending.cbegin();
            // Retrieve the cheepest estimated item.
            index_type current_index = cheepest_it->second; // Make a copy of the index: we are going to modify \c m_pending.
            node_type& current = this->m_traceback[current_index];
            // Mark the current node as no longer in need of processing: this is exactly what we are going to do now.
            this->m_pending.erase(cheepest_it);
            current.open = false;
            current.closed = true;

            // Mark its neighbors as pending.
            std::error_code ec {};
            projector_ref.neighbors(current_index, this->m_temp_neighbors/*, ec*/);
            for (pair_type& item : this->m_temp_neighbors)
            {
                node_type& neighbor = this->m_traceback[item.index];
                if (neighbor.closed) continue; // Skip neighbors that have already been processed.

                cost_type new_cost = current.cost_from_source + item.cost;
                if (!neighbor.open) this->enqueue(item.index, current_index, target, new_cost); // Newly discovered node.
                else if (neighbor.cost_from_source > new_cost) // A better path has been discovered.
                {
                    neighbor.came_from = current_index;
                    neighbor.cost_from_source = new_cost;
                } // else if (...)
            } // for (...)
        } // expand(...)

        void reconstruct_path(const index_type& target, std::vector<index_type>& result) const noexcept
        {
            if (!this->m_traceback[target].closed) return;

            std::vector<index_type> reverse_path {};
            reverse_path.clear();

            const index_type* position_ptr = &target;
            while ((*position_ptr) != this->m_source)
            {
                reverse_path.push_back(*position_ptr);
                position_ptr = &(this->m_traceback[*position_ptr].came_from);
            } // while (...)
            reverse_path.push_back(this->m_source);

            result.clear();
            result.reserve(reverse_path.size());
            for (auto it = reverse_path.crbegin(); it != reverse_path.crend(); ++it) result.push_back(*it);
            result.shrink_to_fit();
        } // reconstruct_path(...)

    public:
        pathfinder() noexcept
        {
            type::traits_check();
            this->m_temp_neighbors.reserve(type::default_neighbor_capacity);
        } // pathfinder(...)
        
        pathfinder(const projector_type& projector, const index_type& source, std::error_code& ec) noexcept
            : m_projector(projector), m_source(source),
            m_traceback(projector.height(), projector.width())
        {
            type::traits_check();
            this->m_temp_neighbors.reserve(type::default_neighbor_capacity);

            if (!this->validate(ec)) this->coerce();
            if (!this->m_traceback.empty()) this->enqueue(source, source, source, 0);
        } // pathfinder(...)
        
        /** @brief Do an exhaustive sweep of the entire grid. */
        void exhaust() noexcept
        {
            while (!this->m_pending.empty()) this->expand(this->m_source);
        } // exhaust(...)

        /** @brief Tries to trace a path to a particular target. */
        void trace(const index_type& target, std::vector<index_type>& result, std::error_code& ec) noexcept
        {
            const std::size_t m = this->m_traceback.height();
            const std::size_t n = this->m_traceback.width();

            if (target.row < 0 || target.row >= m) return aftermath::detail::on_error(ec, std::errc::argument_out_of_domain, "Target must be within the bounds of the surface projection.");
            if (target.column < 0 || target.column >= n) return aftermath::detail::on_error(ec, std::errc::argument_out_of_domain, "Target must be within the bounds of the surface projection.");

            while (!this->m_pending.empty())
            {
                if (this->m_traceback[target].closed) return this->reconstruct_path(target, result);
                this->expand(target);
            } // while (...)

            return aftermath::detail::on_error(ec, std::errc::host_unreachable, "Target unreachable.");
        } // trace(...)
    }; // struct pathfinder
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_PATHFINDER_HPP_INCLUDED
