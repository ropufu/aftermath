
#ifndef ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED

#include "simple_vector.hpp"

#include <array>    // std::array
#include <concepts> // std::default_initializable
#include <cstddef>  // std::size_t
#include <cstring>  // std::memcpy
#include <utility>  // std::swap, std::move

namespace ropufu::aftermath
{
    /** An auxiliary structure to discard old observations. */
    template <std::default_initializable t_value_type,
        typename t_allocator_type = typename simple_vector<t_value_type>::allocator_type>
    struct sliding_vector
    {
        using type = sliding_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;
        
        using container_type = simple_vector<t_value_type, t_allocator_type>;

    private:
        /** Stores twice the number of required elements (at least four) for fast swapping. */
        container_type m_storage;
        /** Points to the current location of the first element. */
        value_type* m_active_ptr;
        /** Points to the location of the first element after the next swap. */
        value_type* m_passive_ptr;
        /** Number of bytes copied internally with every new observation. */
        std::size_t m_memcpy_size;
        /** How far the newest element should be written when calling \c displace_back. */
        std::size_t m_count;

        static constexpr std::size_t storage_size(std::size_t count) noexcept
        {
            // The n items in either active or passive block represent the most
            // recent n observations. Depending on which method is called
            // (displace_back or displance_front) the corresponding item on the
            // left/right of the active block will be overwrtten.
            // [1] -- [n items] -- [1] -- [n items] -- [1]  
            //         ^                   ^                
            //       active             passive             
            switch (count)
            {
                case 0: return 5;
                default: return count + count + 3;
            } // switch (...)
        } // storage_size(...)

        void initialize(std::size_t count)
        {
            std::size_t block_size = (this->m_storage.size() - 3) / 2;

            this->m_active_ptr = this->m_storage.data() + 1;
            this->m_passive_ptr = this->m_storage.data() + block_size + 2;
            this->m_memcpy_size = count * sizeof(value_type);
            this->m_count = count;
        } // initialize(...)

        void deconstruct() noexcept
        {
            this->m_active_ptr = nullptr;
            this->m_passive_ptr = nullptr;
            this->m_memcpy_size = 0;
            this->m_count = 0;
        } // deconstruct(...)

    public:
        /** @brief Creates a vector by stealing from \p other. */
        sliding_vector(type&& other) noexcept
            : m_storage(std::move(other.m_storage))
        {
            this->initialize(other.m_count);
            if (other.m_passive_ptr < other.m_active_ptr) std::swap(this->m_passive_ptr, this->m_active_ptr);
            other.deconstruct();
        } // sliding_vector(...)

        /** @brief Copies a vector by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->m_storage = std::move(other.m_storage);
            this->initialize(other.m_count);
            if (other.m_passive_ptr < other.m_active_ptr) std::swap(this->m_passive_ptr, this->m_active_ptr);
            other.deconstruct();
            return *this;
        } // operator =(...)

        ~sliding_vector() noexcept
        {
            this->deconstruct();
        } // ~sliding_vector(...)

        sliding_vector() noexcept
            : sliding_vector(0)
        {
        } // sliding_vector(...)

        sliding_vector(std::size_t size) noexcept
            : m_storage(type::storage_size(size))
        {
            this->initialize(size);
        } // sliding_vector(...)

        sliding_vector(const type& other) noexcept
            : m_storage(other.m_storage)
        {
            this->initialize(other.size());
            if (other.m_passive_ptr < other.m_active_ptr) std::swap(this->m_passive_ptr, this->m_active_ptr);
        } // sliding_vector(...)

        std::size_t size() const noexcept { return this->m_count; }

        const value_type* data() const noexcept { return this->m_active_ptr; }
        value_type* data() noexcept { return this->m_active_ptr; }

        const value_type* cbegin() const noexcept { return this->m_active_ptr; }
        const value_type* cend() const noexcept { return this->m_active_ptr + this->m_count; }

        const value_type* begin() const noexcept { return this->m_active_ptr; }
        const value_type* end() const noexcept { return this->m_active_ptr + this->m_count; }

        value_type* begin() noexcept { return this->m_active_ptr; }
        value_type* end() noexcept { return this->m_active_ptr + this->m_count; }

        const value_type& operator [](std::size_t index) const
        {
            return *(this->m_active_ptr + index);
        } // operator [](...)

        value_type& operator [](std::size_t index)
        {
            return *(this->m_active_ptr + index);
        } // operator [](...)

        /** Adds another element to the back of the sequence, discarding the first element. */
        void displace_back(const value_type& value) noexcept
        {
            // Copy newest element to the right of active block.
            std::memcpy(this->m_active_ptr + this->m_count, &value, sizeof(value_type));
            // Copy the data discarding the leftmost element.
            std::memcpy(this->m_passive_ptr, this->m_active_ptr + 1, this->m_memcpy_size);
            // Mark new location as active.
            std::swap(this->m_passive_ptr, this->m_active_ptr);
        } // displace_back(...)

        /** Adds another element to the front of the sequence, discarding the last element. */
        void displace_front(const value_type& value) noexcept
        {
            // Copy newest element to the left of active block.
            std::memcpy(this->m_active_ptr - 1, &value, sizeof(value_type));
            // Copy the data discarding the rightmost element.
            std::memcpy(this->m_passive_ptr, this->m_active_ptr - 1, this->m_memcpy_size);
            // Mark new location as active.
            std::swap(this->m_passive_ptr, this->m_active_ptr);
        } // displace_front(...)
    }; // struct sliding_vector

    /** An auxiliary structure to discard old observations. */
    template <std::default_initializable t_value_type, std::size_t t_size>
    struct sliding_array : public std::array<t_value_type, t_size>
    {
        using type = sliding_array<t_value_type, t_size>;
        using base_type = std::array<t_value_type, t_size>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the back of the sequence, discarding the first element. */
        void displace_back(const value_type& value) noexcept
        {
            constexpr std::size_t tail_size = (t_size - 1);
            // Shift the data to the left.
            std::memmove(this->data(), this->data() + 1, tail_size * sizeof(value_type));
            // Overwrite last element.
            std::memcpy(this->data() + tail_size, &value, sizeof(value_type));
        } // displace_back(...)

        /** Adds another element to the front of the sequence, discarding the last element. */
        void displace_front(const value_type& value) noexcept
        {
            constexpr std::size_t tail_size = (t_size - 1);
            // Shift the data to the right.
            std::memmove(this->data() + 1, this->data(), tail_size * sizeof(value_type));
            // Overwrite first element.
            std::memcpy(this->data(), &value, sizeof(value_type));
        } // displace_front(...)
    }; // struct sliding_array

    /** Simple time window with two observations. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 2> : public std::array<t_value_type, 2>
    {
        using type = sliding_array<t_value_type, 2>;
        using base_type = std::array<t_value_type, 2>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the back of the sequence, discarding the first element. */
        void displace_back(const value_type& value) noexcept
        {
            std::memcpy(this->data(), this->data() + 1, sizeof(value_type));
            std::memcpy(this->data() + 1, &value, sizeof(value_type));
        } // displace_back(...)

        /** Adds another element to the front of the sequence, discarding the last element. */
        void displace_front(const value_type& value) noexcept
        {
            std::memcpy(this->data() + 1, this->data(), sizeof(value_type));
            std::memcpy(this->data(), &value, sizeof(value_type));
        } // displace_front(...)
    }; // struct sliding_array

    /** Simple time window with one observation. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 1> : public std::array<t_value_type, 1>
    {
        using type = sliding_array<t_value_type, 1>;
        using base_type = std::array<t_value_type, 1>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the back of the sequence, discarding the first element. */
        void displace_back(const value_type& value) noexcept
        {
            std::memcpy(this->data(), &value, sizeof(value_type));
        } // displace_back(...)

        /** Adds another element to the front of the sequence, discarding the last element. */
        void displace_front(const value_type& value) noexcept
        {
            std::memcpy(this->data(), &value, sizeof(value_type));
        } // displace_front(...)
    }; // struct sliding_array

    /** Trivial time window. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 0> : public std::array<t_value_type, 0>
    {
        using type = sliding_array<t_value_type, 0>;
        using base_type = std::array<t_value_type, 0>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the back of the sequence, discarding the first element. */
        void displace_back(const value_type& /*value*/) noexcept { }
        
        /** Adds another element to the front of the sequence, discarding the last element. */
        void displace_front(const value_type& value) noexcept { }
    }; // struct sliding_array
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED
