
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED

#include "observer.hpp"

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

        using observer_type = observer<value_type, container_type>;

    private:
        /** Number of observations generated. */
        std::size_t m_count = 0;
        std::vector<observer_type*> m_observer_pointers = {};

    protected:
        /** Called when the process should be cleared. */
        virtual void on_clear() noexcept = 0;

        /** Called when a single observation is to be generated. */
        virtual value_type on_next() noexcept = 0;

        /** Called when a block of observations is to be generated. */
        virtual void on_next(container_type& values) noexcept = 0;

    public:
        virtual ~discrete_process() noexcept = default;

        bool try_register_observer(observer_type* observer_ptr) noexcept
        {
            if (observer_ptr == nullptr) return false; // This observer is not an observer.
            for (observer_type* x : this->m_observer_pointers)
                if (x == observer_ptr)
                    return false; // This observer has already been registered.
            this->m_observer_pointers.push_back(observer_ptr);
            return true;
        } // try_register_observer(...)

        void clear_observers() noexcept
        {
            this->m_observer_pointers.clear();
        } // clear_observers(...)

        /** Purges past observations. Observers are left intact. */
        void clear() noexcept
        {
            this->on_clear();
            this->m_count = 0;
            for (observer_type* x : this->m_observer_pointers) x->reset();
        } // clear(...)

        /** Number of observations generated so far. */
        std::size_t count() const noexcept { return this->m_count; }

        /** Generate a single observation. */
        value_type next() noexcept
        {
            value_type result = this->on_next();
            ++this->m_count;
            for (observer_type* x : this->m_observer_pointers) x->observe(result);
            return result;
        } // next(...)

        /** Generate a block of observation. */
        void next(container_type& values) noexcept
        {
            this->on_next(values);
            this->m_count += std::ranges::size(values);
            for (observer_type* x : this->m_observer_pointers) x->observe(values);
        } // next(...)
    }; // struct discrete_process
} // namespace ropufu::aftermath::sequential

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_DISCRETE_PROCESS_HPP_INCLUDED
