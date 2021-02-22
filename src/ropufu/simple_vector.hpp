
#ifndef ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED

#include "concepts.hpp"

#include <concepts>  // std::default_initializable
#include <cstddef>   // std::size_t, std::nullptr_t
#include <cstring>   // std::memset, std::memcpy
#include <memory>    // std::allocator, std::allocator_traits
#include <ranges>    // std::ranges:range
#include <stdexcept> // std::out_of_range, std::logic_error
#include <type_traits> // std::is_arithmetic_v
#include <utility>   // std::move, std::hash

namespace ropufu::aftermath
{
    namespace detail
    {
        template <typename t_derived_type, typename t_value_type, typename t_size_type>
        struct vector_wipe_module
        {
            using derived_type = t_derived_type;
            using value_type = t_value_type;
            using size_type = t_size_type;

            static constexpr bool is_enabled = false;
        }; // struct vector_wipe_module

        template <typename t_derived_type, typename t_value_type, typename t_size_type>
            requires (std::is_arithmetic_v<t_value_type>)
        struct vector_wipe_module<t_derived_type, t_value_type, t_size_type>
        {
            using derived_type = t_derived_type;
            using value_type = t_value_type;
            using size_type = t_size_type;
            
            static constexpr bool is_enabled = true;

            /** @brief Overwrites allocated memory with zeros.
             *  @warning No destructors are called.
             */
            void wipe() noexcept
            {
                derived_type* that = static_cast<derived_type*>(this);
                std::memset(that->m_begin_ptr, 0, static_cast<std::size_t>(that->m_size) * sizeof(value_type));
            } // wipe(...)

            /** @brief Overwrites allocated memory with that from \param other.
             *  @warning No size checks are performed.
             *  @warning No constructors are called.
             */
            void overwrite(const derived_type& other) noexcept
            {
                derived_type* that = static_cast<derived_type*>(this);
                std::memcpy(that->m_begin_ptr, other.m_begin_ptr, static_cast<std::size_t>(that->m_size) * sizeof(value_type));
            } // overwrite(...)

            /** Creates an uninitialized vector. */
            static derived_type uninitialized(size_type size) noexcept
            {
                return derived_type(nullptr, size);
            } // uninitialized(...)
        }; // struct vector_wipe_module<...>
    } // namespace detail

    /** @brief A sequence of elements stored contiguously in memory. */
    template <std::default_initializable t_value_type, typename t_allocator_type = std::allocator<t_value_type>>
    struct simple_vector;

    template <std::default_initializable t_value_type, typename t_allocator_type>
    struct simple_vector
        : public detail::vector_wipe_module<
            simple_vector<t_value_type, t_allocator_type>,
            t_value_type,
            typename std::allocator_traits<t_allocator_type>::size_type>
    {
        using type = simple_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

        using allocator_traits_type = std::allocator_traits<allocator_type>;
        using size_type = typename allocator_traits_type::size_type;
        using iterator_type = value_type*;
        using const_iterator_type = const value_type*;

        using wipe_module = detail::vector_wipe_module<type, value_type, size_type>;

        template <std::default_initializable, typename> friend struct simple_vector;
        template <typename, typename, typename> friend struct detail::vector_wipe_module;

    private:
        allocator_type m_allocator = {};
        size_type m_size = 0; // Number of elements in the sequence.
        value_type* m_begin_ptr = nullptr; // Pointer to the allocated memory.

        /** @brief Creates an uninitialized vector.
         *  @exception std::bad_alloc Allocation failed.
         */
        simple_vector(std::nullptr_t, size_type size)
            : m_size(size)
        {
            this->allocate();
        } // simple_vector(...)

        /** @brief Allocates memory for storing elements.
         *  @exception std::bad_alloc Allocation failed.
         *  @exception std::logic_error Deallocation has to be called first.
         */
        void allocate()
        {
            if (this->m_begin_ptr != nullptr) throw std::logic_error("Memory has to be deallocated prior to another allocation call.");
            //if (this->m_size == 0) return; // Do not perform trivial allocations.
            this->m_begin_ptr = this->m_allocator.allocate(this->m_size);
        } // allocate(...)

        /** @brief Relinquish and clean-up allocated memory. */
        void deconstruct() noexcept
        {
            if (this->m_begin_ptr == nullptr) return; // Check if memory has already been deallocated.

            if constexpr (!wipe_module::is_enabled)
            {
                value_type* end_ptr = this->m_begin_ptr + this->m_size;
                for (value_type* it = this->m_begin_ptr; it != end_ptr; ++it) allocator_traits_type::destroy(this->m_allocator, it);
            } // if constexpr (...)

            this->m_allocator.deallocate(this->m_begin_ptr, this->m_size);
            this->m_begin_ptr = nullptr;
        } // deconstruct(...)

    public:
        /** @brief Creates a vector by stealing from \p other. */
        simple_vector(type&& other) noexcept
            : m_allocator(std::move(other.m_allocator)), m_size(other.m_size), m_begin_ptr(other.m_begin_ptr)
        {
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
        } // simple_vector(...)

        /** @brief Copies a vector by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->deconstruct(); // Relinquish existing memory.
            // Reset current fields.
            this->m_allocator = std::move(other.m_allocator);
            this->m_size = other.m_size;
            this->m_begin_ptr = other.m_begin_ptr;
            // Clear stolen references.
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
            return *this;
        } // operator =(...)

        ~simple_vector() noexcept { this->deconstruct(); }

        /** @brief Creates an empty vector. */
        simple_vector() : simple_vector(nullptr, 0) { }

        /** @brief Creates an empty (default-constructed) vector of a given size. */
        explicit simple_vector(size_type size) : simple_vector(nullptr, size)
        {
            if constexpr (wipe_module::is_enabled) this->wipe();
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
        template <std::ranges::range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        explicit simple_vector(const t_container_type& container) : simple_vector(nullptr, container.size())
        {
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
            if constexpr (wipe_module::is_enabled) this->overwrite(other);
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
                this->deconstruct(); // Relinquish existing memory.
                this->m_size = other.m_size;
                this->allocate(); // Allocate memory to hold new values.
            } // if (...)

            if constexpr (wipe_module::is_enabled) this->overwrite(other);
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

        std::size_t get_hash() const noexcept
        {
            std::size_t result = 0;
            std::hash<value_type> value_hash = {};
            const value_type* end_ptr = this->m_begin_ptr + this->m_size;
            for (const value_type* it = this->m_begin_ptr; it != end_ptr; ++it)
            {
                result ^= value_hash(*it);
                result <<= 1;
            } // for (...)
            return result;
        } // get_hash(...)
    }; // struct simple_vector
} // namespace ropufu::aftermath

namespace std
{
    template <std::default_initializable t_value_type, typename t_allocator_type>
    struct hash<ropufu::aftermath::simple_vector<t_value_type, t_allocator_type>>
    {
        using argument_type = ropufu::aftermath::simple_vector<t_value_type, t_allocator_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            return x.get_hash();
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_SIMPLE_VECTOR_HPP_INCLUDED
