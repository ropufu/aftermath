
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_SCALAR_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_SCALAR_PROCESS_HPP_INCLUDED

#include <concepts> // std::totally_ordered, std::same_as
#include <cstddef>  // std::size_t
#include <ranges>   // std::ranges::...

namespace ropufu::aftermath::sequential
{
    template <std::totally_ordered t_value_type>
    struct scalar_process
    {
        using type = scalar_process<t_value_type>;
        using value_type = t_value_type;

    private:
        std::size_t m_count = 0;

    protected:
        /** Called when the process should be cleared. Count has already set to zero. */
        virtual void on_clear() noexcept = 0;

        /** Called when a single observation is to be generated. Count has already been updated. */
        virtual value_type on_next() noexcept = 0;

        /** Called when a block of observations is to be generated. Count has already been updated. */
        template <std::ranges::random_access_range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        virtual void on_next(t_container_type& values) noexcept = 0;

    public:
        /** Purges past observations. */
        void clear() noexcept
        {
            this->m_count = 0;
            this->on_clear();
        } // clear(...)

        /** Number of observations generated so far. */
        std::size_t count() const noexcept { return this->m_count; }

        /** Generate a single observation. */
        value_type next() noexcept
        {
            ++this->m_count;
            return this->on_next();
        } // next(...)

        /** Generate a block of observation. */
        template <std::ranges::random_access_range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        void next(t_container_type& values) noexcept
        {
            this->m_count += std::ranges::size(values);
            this->on_next(values);
        } // next(...)
    }; // struct scalar_process
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_SCALAR_PROCESS_HPP_INCLUDED
