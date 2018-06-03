
#ifndef ROPUFU_AFTERMATH_ALGORITHM_MATRIX_PATHFINDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGORITHM_MATRIX_PATHFINDER_HPP_INCLUDED

#include "../algebra/matrix_index.hpp" // algebra::matrix_index
#include "../algebra/matrix.hpp" // algebra::matrix

#include <cstddef>   // std::size_t
#include <unordered_set> // std::unordered_set
#include <vector>    // std::vector

namespace ropufu::aftermath::algorithm
{
    /** @brief Traces a shortest path on a Boolean matrix from one index to another.
     *         Inspired by the A-star algorithm.
     *         \c true elements of the matrix represent walkable cells.
     *         \c false elements of the matrix represent blocked cells.
     *  @reference https://en.wikipedia.org/wiki/A*_search_algorithm.
     */
    struct matrix_pathfinder
    {
        using type = matrix_pathfinder;

        using index_type = ropufu::aftermath::algebra::matrix_index<std::size_t>;
        using mask_type = ropufu::aftermath::algebra::matrix<bool>;
        using cost_type = ropufu::aftermath::algebra::matrix<std::size_t>;
        using traceback_type = ropufu::aftermath::algebra::matrix<char>;

        static constexpr char direction_top = 1;
        static constexpr char direction_right = 2;
        static constexpr char direction_bottom = 3;
        static constexpr char direction_left = 4;

        /** L1 distance between two indices. */
        static std::size_t distance(const index_type& a, const index_type& b) noexcept
        {
            std::size_t dx = (a.column < b.column) ? (b.column - a.column) : (a.column - b.column);
            std::size_t dy = (a.row < b.row) ? (b.row - a.row) : (a.row - b.row);
            return dx + dy;
        } // distance(...)

    private:
        mask_type m_mask = {}; // Indicates walkable nodes: 1 for walkable, 0 for blocked.
        index_type m_source = {}; // The starting point of the path.
        // ~~ Auxiliary members ~~
        std::unordered_set<index_type> m_pending = {}; // The set of currently discovered nodes that are not evaluated yet, a.k.a. "open set".
        mask_type m_visited = {}; // Indicates nodes that have already been evaluated, a.k.a. "closed set".
        cost_type m_cost_from_start = {}; // For each node, the cost of getting from the start node to that node, a.k.a. "g score".
        traceback_type m_came_from = {}; // For each node, which neighbor node it can most efficiently be reached from.

        static constexpr std::size_t max_cost(std::size_t height, std::size_t width) noexcept
        {
            return height * width + 1;
        } // max_cost(...)

        void initialize() noexcept
        {
            if (this->m_source.row >= this->m_mask.height()) return;
            if (this->m_source.column >= this->m_mask.width()) return;
            if (!this->m_mask[this->m_source]) return; // Initial cell is not walkable: there is nothing to be done.

            this->m_pending.insert(this->m_source);
            // this->m_cost_from_start[this->m_source] = 0; // No need to set the cost, as it defaults to zero.
        } // initialize(...)

    public:
        matrix_pathfinder() noexcept { }
        
        matrix_pathfinder(const mask_type& grid, const index_type& source) noexcept
            : m_mask(grid), m_source(source),
            m_visited(grid.height(), grid.width()),
            m_cost_from_start(grid.height(), grid.width()),
            m_came_from(grid.height(), grid.width())
        {
            this->initialize();
        } // matrix_pathfinder(...)

        template <typename t_value_type, bool t_is_row_major, typename t_predicate_type>
        matrix_pathfinder(
            const ropufu::aftermath::algebra::matrix<t_value_type, t_is_row_major>& grid, const index_type& source, t_predicate_type&& predicate) noexcept
            : m_mask(grid.height(), grid.width()), m_source(source),
            m_visited(grid.height(), grid.width()),
            m_cost_from_start(grid.height(), grid.width()),
            m_came_from(grid.height(), grid.width())
        {
            const std::size_t m = grid.height();
            const std::size_t n = grid.width();

            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t j = 0; j < n; ++j)
                    this->m_mask(i, j) = predicate(grid(i, j));
            
            this->initialize();
        } // matrix_pathfinder(...)

        template <typename t_grid_type, typename t_filter_type>
        matrix_pathfinder(
            const t_grid_type& grid, const index_type& source, t_filter_type&& filter) noexcept
            : m_mask(grid.height(), grid.width()), m_source(source),
            m_visited(grid.height(), grid.width()),
            m_cost_from_start(grid.height(), grid.width()),
            m_came_from(grid.height(), grid.width())
        {
            const std::size_t m = grid.height();
            const std::size_t n = grid.width();

            for (std::size_t i = 0; i < m; ++i)
                for (std::size_t j = 0; j < n; ++j)
                    this->m_mask(i, j) = filter(i, j);
            
            this->initialize();
        } // matrix_pathfinder(...)

        /** @brief Longest possible path. */
        std::size_t capacity() const noexcept { return this->m_mask.size(); }

        // /** @brief Do an exhaustive sweep of the entire grid. */
        // void exhause() noexcept
        // {
        //     /** @todo Implement! */
        // } // exhaust(...)

        /** @brief Tries to trace a path to a particular target. */
        bool try_trace(const index_type& target, std::vector<index_type>& result) noexcept
        {
            std::size_t m = this->m_mask.height();
            std::size_t n = this->m_mask.width();

            const std::size_t max_cost = type::max_cost(m, n);

            if (target.row >= m) return false; // Row index out of range.
            if (target.column >= n) return false; // Column index out of range.
            if (!this->m_mask[target]) return false; // The terminal point is blocked.
            if (this->m_visited[target]) return this->reconstruct_path(target, result);

            while (!this->m_pending.empty())
            {
                // First, take the item with smallest estimated cost to target.
                const index_type* current_ptr = nullptr;
                std::size_t cheapest_estimate = max_cost;
                std::size_t current_cost = max_cost;
                for (const index_type& x : this->m_pending)
                {
                    std::size_t cost = this->m_cost_from_start[x];
                    std::size_t estimated_cost = cost + type::distance(x, target);
                    if (estimated_cost < cheapest_estimate)
                    {
                        cheapest_estimate = estimated_cost;
                        current_cost = cost;
                        current_ptr = &x;
                    } // if (...)
                } // for (...)
                index_type current = *(current_ptr); // Make a copy: we are going to remove it from its container.

                // Terminal condition.
                if (current == target) return this->reconstruct_path(target, result);

                // Second, mark currect node as visited.
                this->m_pending.erase(current);
                this->m_visited[current] = true;

                // Third, mark its neighbors as pending.
                if (current.row > 0)        this->process_neighbor(current.row - 1, current.column, type::direction_bottom, current_cost + 1); // Top.
                if (current.column < n - 1) this->process_neighbor(current.row, current.column + 1, type::direction_left, current_cost + 1); // Right.
                if (current.row < m - 1)    this->process_neighbor(current.row + 1, current.column, type::direction_top, current_cost + 1); // Bottom.
                if (current.column > 0)     this->process_neighbor(current.row, current.column - 1, type::direction_right, current_cost + 1); // Left.
            } // while (...)
            return false;
        } // trace(...)

    private:
        void process_neighbor(std::size_t row_index, std::size_t column_index, char traceback_direction, std::size_t cost) noexcept
        {
            if (this->m_visited(row_index, column_index)) return; // Skip neighbors that have already been processed.
            if (!this->m_mask(row_index, column_index)) return; // This path is blocked, skip this neighbor.

            index_type index{ row_index, column_index };
            auto search = this->m_pending.find(index);
            if (search == this->m_pending.end()) this->m_pending.insert(index); // Neighbor hasn't been discovered yest: mark for processing.
            else if (cost >= this->m_cost_from_start[index]) return; // Otherwise, make sure that the current cost is smaller than the know one.

            // This path is the best so far.
            this->m_came_from[index] = traceback_direction;
            this->m_cost_from_start[index] = cost;
        } // process_neighbor(...)

        bool reconstruct_path(const index_type& target, std::vector<index_type>& result) const noexcept
        {
            result.clear();
            result.reserve(this->m_cost_from_start[target] + 1);

            result.push_back(target);

            index_type position = target;
            char direction = this->m_came_from[position];
            while (direction != 0)
            {
                switch (direction)
                {
                case type::direction_top: --position.row; break;
                case type::direction_right: ++position.column; break;
                case type::direction_bottom: ++position.row; break;
                case type::direction_left: --position.column; break;
                default: return false;
                }

                result.push_back(position);                
                direction = this->m_came_from[position];
            } // while (...)

            result.shrink_to_fit();
            return true;
        } // reconstruct_path(...)
    }; // struct matrix_pathfinder
} // namespace ropufu::aftermath::algorithm

#endif // ROPUFU_AFTERMATH_ALGORITHM_MATRIX_PATHFINDER_HPP_INCLUDED
