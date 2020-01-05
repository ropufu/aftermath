
#ifndef ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED

#include "type_traits.hpp"

#include <cstddef>   // std::size_t, std::nullptr_t
#include <cstring>   // std::memset, std::memcpy
#include <memory>    // std::allocator, std::allocator_traits
#include <stdexcept> // std::out_of_range, std::logic_error
#include <type_traits> // ...
#include <utility>     // std::move

namespace ropufu::aftermath
{
    namespace detail
    {
        template <bool t_is_enabled, typename t_derived_type, typename t_value_type, typename t_size_type>
        struct vector_wipe_module
        {
            using derived_type = t_derived_type;
            using value_type = t_value_type;
            using size_type = t_size_type;
        }; // struct vector_wipe_module

        template <typename t_derived_type, typename t_value_type, typename t_size_type>
        struct vector_wipe_module<true, t_derived_type, t_value_type, t_size_type>
        {
            using derived_type = t_derived_type;
            using value_type = t_value_type;
            using size_type = t_size_type;

            /** @brief Overwrites allocated memory with zeros.
             *  @warning No destructors are called.
             */
            void wipe() noexcept
            {
                derived_type* that = static_cast<derived_type*>(this);
                std::memset(that->m_begin_ptr, 0, static_cast<std::size_t>(that->m_size) * sizeof(value_type));
            } // wipe(...)

            /** Creates an uninitialized vector. */
            static derived_type uninitialized(size_type size) noexcept
            {
                return derived_type(nullptr, size);
            } // uninitialized(...)
        }; // struct vector_wipe_module<...>
    } // namespace detail

    /** @brief A sequence of elements stored contiguously in memory. */
    template <typename t_value_type, typename t_allocator_type = std::allocator<t_value_type>>
    struct simple_vector;

    template <typename t_value_type, typename t_allocator_type>
    struct simple_vector
        : public detail::vector_wipe_module<std::is_arithmetic_v<t_value_type>,
            simple_vector<t_value_type, t_allocator_type>, t_value_type,
            typename std::allocator_traits<t_allocator_type>::size_type>
    {
        using type = simple_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

        using allocator_traits_type = std::allocator_traits<allocator_type>;
        using size_type = typename allocator_traits_type::size_type;
        using iterator_type = value_type*;
        using const_iterator_type = const value_type*;

        static constexpr bool is_trivial =
            std::is_trivially_constructible_v<value_type> &&
            std::is_trivially_destructible_v<value_type>;

        template <typename, typename> friend struct simple_vector;
        template <bool, typename, typename, typename> friend struct detail::vector_wipe_module;

    private:
        allocator_type m_allocator = {};
        size_type m_size = 0; // Number of elements in the sequence.
        value_type* m_begin_ptr = nullptr; // Pointer to the allocated memory.

        static constexpr void traits_check() noexcept
        {
            static_assert(std::is_default_constructible_v<value_type>, "value_type has to be default constructible.");
        } // traits_check(...)

        /** @brief Creates an uninitialized vector.
         *  @exception std::bad_alloc Allocation failed.
         */
        simple_vector(std::nullptr_t, size_type size)
            : m_size(size)
        {
            type::traits_check();
            this->allocate();
        } // simple_vector(...)

        /** @brief Allocates memory for storing elements.
         *  @exception std::bad_alloc Allocation failed.
         *  @exception std::logic_error Deallocation has to be called first.
         */
        void allocate()
        {
            if (this->m_begin_ptr != nullptr) throw std::logic_error("Memory has to be deallocated prior to another allocation call.");
            if (this->m_size == 0) return; // Do not perform trivial allocations.
            this->m_begin_ptr = this->m_allocator.allocate(this->m_size);
        } // allocate(...)

        /** @brief Relinquish allocated memory. */
        void deallocate() noexcept
        {
            if (this->m_begin_ptr == nullptr) return; // Check if memory has already been deallocated.

            if constexpr (!type::is_trivial)
            {
                value_type* end_ptr = this->m_begin_ptr + this->m_size;
                for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it) allocator_traits_type::destroy(this->m_allocator, it);
            } // if constexpr (...)

            this->m_allocator.deallocate(this->m_begin_ptr, this->m_size);
            this->m_begin_ptr = nullptr;
        } // deallocate(...)

    public:
        /** @brief Creates a vector by stealing from \p other. */
        simple_vector(type&& other) noexcept
            : m_allocator(std::move(other.m_allocator)), m_size(other.m_size), m_begin_ptr(other.m_begin_ptr)
        {
            type::traits_check();
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
        } // simple_vector(...)

        /** @brief Copies a vector by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->deallocate(); // Relinquish existing memory.
            // Reset current fields.
            this->m_allocator = std::move(other.m_allocator);
            this->m_size = other.m_size;
            this->m_begin_ptr = other.m_begin_ptr;
            // Clear stolen references.
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
            return *this;
        } // operator =(...)

        ~simple_vector() noexcept { this->deallocate(); }

        /** @brief Creates an empty vector. */
        simple_vector() noexcept { type::traits_check(); }

        /** @brief Creates an empty (default-constructed) vector of a given size. */
        explicit simple_vector(size_type size) : simple_vector(nullptr, size)
        {
            type::traits_check();
            if constexpr (std::is_arithmetic_v<value_type>)
                std::memset(this->m_begin_ptr, 0, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                value_type* end_ptr = this->m_begin_ptr + this->m_size;
                for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it)
                    allocator_traits_type::construct(this->m_allocator, it);
            } // if constexpr (...)
        } // simple_vector(...)

        /** @brief Creates a vector with all entries set to \p value. */
        simple_vector(size_type size, const value_type& value) : simple_vector(nullptr, size)
        {
            value_type* end_ptr = this->m_begin_ptr + this->m_size;
            for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it)
                allocator_traits_type::construct(this->m_allocator, it, value);
        } // simple_vector(...)

        /** @brief Creates a vector from another sequence. */
        template <typename t_container_type, typename t_void_type = std::enable_if_t<
            aftermath::type_traits::is_iterable_v<t_container_type> &&
            std::is_same_v<typename t_container_type::value_type, value_type>>>
        explicit simple_vector(const t_container_type& container) : simple_vector(nullptr, container.size())
        {
            type::traits_check();
            value_type* it = this->m_begin_ptr;
            for (const value_type& x : container)
            {
                allocator_traits_type::construct(this->m_allocator, it, x);
                ++it;
            } // for (...)
        } // simple_vector(...)

        /** @brief Creates a vector as a copy of another vector. */
        simple_vector(const type& other) : simple_vector(nullptr, other.m_size)
        {
            if constexpr (type::is_trivial)
                std::memcpy(this->m_begin_ptr, other.m_begin_ptr, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                const value_type* other_it = other.m_begin_ptr;
                value_type* end_ptr = this->m_begin_ptr + this->m_size;
                for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it)
                {
                    allocator_traits_type::construct(this->m_allocator, it, *(other_it));
                    ++other_it;
                } // for (...)
            } // if constexpr (...)
        } // simple_vector(...)

        /** @brief Creates a vector as a copy of another vector by casting its underlying values. */
        template <typename t_other_value_type, typename t_other_allocator_type>
        explicit operator simple_vector<t_other_value_type, t_other_allocator_type>() const
        {
            simple_vector<t_other_value_type, t_other_allocator_type> other { nullptr, this->m_size };
            const value_type* it = this->m_begin_ptr;
            t_other_value_type* other_end_ptr = other.m_begin_ptr + other.m_size;
            for (t_other_value_type* other_it = other.m_begin_ptr; other_it != other_end_ptr; ++other_it)
            {
                std::allocator_traits<t_other_allocator_type>::construct(other.m_allocator, other_it, static_cast<t_other_value_type>(*(it)));
                ++it;
            } // for (...)
            return other;
        } // static_cast<...>

        /** @brief Overwrites the vector with values from \p other. */
        type& operator =(const type& other) noexcept
        {
            if (this == &other) return *this; // Do nothing if this is self-assignment.

            // Re-allocate memory if vectors are incompatible.
            if (this->m_size != other.m_size)
            {
                this->deallocate(); // Relinquish existing memory.
                this->m_size = other.m_size;
                this->allocate(); // Allocate memory to hold new values.
            } // if (...)

            if constexpr (type::is_trivial)
                std::memcpy(this->m_begin_ptr, other.m_begin_ptr, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                const value_type* other_it = other.m_begin_ptr;
                value_type* end_ptr = this->m_begin_ptr + this->m_size;
                for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it)
                {
                    (*it) = (*other_it);
                    ++other_it;
                } // for (...)
                return *this;
            } // if constexpr (...)

            return *this;
        } // operator =(...)

        /** @brief Overwrites each entry of the vector with \p value. */
        type& operator =(const value_type& value) noexcept
        {
            this->fill(value);
            return *this;
        } // operator =(...)

        /** Fills vector with \p value. */
        void fill(const value_type& value) noexcept
        {
            value_type* end_ptr = this->m_begin_ptr + this->m_size;
            for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it) (*it) = value;
        } // fill(...)

        /** Number of elements in the vector. */
        size_type size() const noexcept { return this->m_size; }

        /** Checks if the vector is empty. */
        bool empty() const noexcept { return this->m_size == 0; }

        const value_type* data() const noexcept { return this->m_begin_ptr; }
        value_type* data() noexcept { return this->m_begin_ptr; }

        /** Access the first element. */
        const value_type& front() const noexcept { return *(this->m_begin_ptr); }
        /** Access the first element. */
        value_type& front() noexcept { return *(this->m_begin_ptr); }

        /** Access the last element. */
        const value_type& back() const noexcept { return *((this->m_begin_ptr + this->m_size) - 1); }
        /** Access the last element. */
        value_type& back() noexcept { return *((this->m_begin_ptr + this->m_size) - 1); }

        const_iterator_type cbegin() const noexcept { return this->m_begin_ptr; }
        const_iterator_type cend() const noexcept { return this->m_begin_ptr + this->m_size; }

        const_iterator_type begin() const noexcept { return this->m_begin_ptr; }
        const_iterator_type end() const noexcept { return this->m_begin_ptr + this->m_size; }

        iterator_type begin() noexcept { return this->m_begin_ptr; }
        iterator_type end() noexcept { return this->m_begin_ptr + this->m_size; }

        /** @brief Access vector elements. No bound checks are performed. */
        const value_type& operator [](size_type index) const { return this->m_begin_ptr[index]; }
        /** @brief Access vector elements. No bound checks are performed. */
        value_type& operator [](size_type index) { return this->m_begin_ptr[index]; }

        /** @brief Access vector elements.
         *  @exception std::out_of_range Index outside the dimensions of the vector.
         */
        const value_type& at(size_type index) const
        {
            if (index >= this->m_size) throw std::out_of_range("Index must be less than the size of the vector.");
            return this->operator [](index);
        } // at(...)
        /** @brief Access vector elements.
         *  @exception std::out_of_range Index outside the dimensions of the vector.
         */
        value_type& at(size_type index)
        {
            if (index >= this->m_size) throw std::out_of_range("Index must be less than the size of the vector.");
            return this->operator [](index);
        } // at(...)
    }; // struct simple_vector
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED
