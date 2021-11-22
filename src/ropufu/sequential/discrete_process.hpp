
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED

#include <concepts> // std::same_as
#include <cstddef>  // std::size_t
#include <ranges>   // std::ranges::...
#include <vector>   // std::vector

namespace ropufu::aftermath::sequential
{
    template <typename t_value_type, std::ranges::random_access_range t_container_type>
        requires std::same_as<std::ranges::range_value_t<t_container_type>, t_value_type>
    struct discrete_process
    {
        using type = discrete_process<t_value_type, t_container_type>;
        using value_type = t_value_type;
        using container_type = t_container_type;

    private:
        /** Number of observations generated. */
        std::size_t m_count = 0;

    protected:
        /** Called when the process should be cleared. */
        virtual void on_clear() noexcept = 0;

        /** Called when a single observation is to be generated. */
        virtual value_type on_next() noexcept = 0;

        /** Called when a block of observations is to be generated. */
        virtual void on_next(container_type& values) noexcept = 0;

    public:
        discrete_process() noexcept = default;

        virtual ~discrete_process() noexcept = default;

        /** Purges past observations. Observers are left intact. */
        void clear() noexcept
        {
            this->m_count = 0;
        } // clear(...)

        /** Number of observations generated so far. */
        std::size_t count() const noexcept { return this->m_count; }

        /** Generate a single observation. */
        value_type next() noexcept
        {
            value_type result = this->on_next();
            ++this->m_count;
            return result;
        } // next(...)

        /** Generate a block of observation. */
        void next(container_type& values) noexcept
        {
            this->on_next(values);
            this->m_count += std::ranges::size(values);
        } // next(...)

        value_type operator ()() noexcept { return this->next(); }

        void operator ()(container_type& values) noexcept { this->next(values); }
    }; // struct discrete_process
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED
