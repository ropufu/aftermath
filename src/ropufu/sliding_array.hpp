
#ifndef ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED

#include "simple_vector.hpp"

#include <array>    // std::array
#include <concepts> // std::default_initializable
#include <cstddef>  // std::size_t
#include <cstring>  // std::memmove
#include <utility>  // std::swap, std::move

namespace ropufu::aftermath
{
    /** An auxiliary structure to discard old observations. */
    template <std::default_initializable t_value_type,
        typename t_allocator_type = typename simple_vector<t_value_type>::allocator_type>
    struct sliding_vector : public simple_vector<t_value_type, t_allocator_type>
    {
        using type = sliding_vector<t_value_type, t_allocator_type>;
        using base_type = simple_vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

    private:
        value_type m_guard;
        value_type* m_destination_ptr = nullptr;
        value_type* m_source_ptr = nullptr;
        value_type* m_back_ptr = nullptr;
        std::size_t m_tail_memory_size = 0;

        void initialize()
        {
            switch (this->size())
            {
            case 0:
                this->m_destination_ptr = &this->m_guard;
                this->m_source_ptr = &this->m_guard;
                this->m_back_ptr = &this->m_guard;
                this->m_tail_memory_size = 0;
                break;
            case 1:
                this->m_destination_ptr = &this->m_guard;
                this->m_source_ptr = &this->m_guard;
                this->m_back_ptr = this->data();
                this->m_tail_memory_size = 0;
                break;
            default:
                this->m_destination_ptr = this->data();
                this->m_source_ptr = this->data() + 1;
                this->m_back_ptr = this->data() + (this->size() - 1);
                this->m_tail_memory_size = (this->size() - 1) * sizeof(value_type);
                break;
            } // switch (...)
        } // initialize(...)

        void deconstruct() noexcept
        {
            this->m_destination_ptr = nullptr;
            this->m_source_ptr = nullptr;
            this->m_back_ptr = nullptr;
            this->m_tail_memory_size = 0;
        } // deconstruct(...)

    public:
        ~sliding_vector() noexcept { this->deconstruct(); }

        /** @brief Creates a vector by stealing from \p other. */
        sliding_vector(type&& other) noexcept
            : base_type(std::forward<base_type>(other))
        {
            other.deconstruct();
            this->initialize();
        } // sliding_vector(...)

        /** @brief Copies a vector by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->deconstruct();
            static_cast<base_type&>(*this) = std::forward<base_type>(other);
            this->initialize();
            other.deconstruct();
            return *this;
        } // operator =(...)

        sliding_vector() noexcept
            : base_type()
        {
            this->initialize();
        } // sliding_vector(...)

        sliding_vector(std::size_t size) noexcept
            : base_type(size)
        {
            this->initialize();
        } // sliding_vector(...)

        /** Adds another element to the observed sequence. */
        void shift_back(const value_type& value) noexcept
        {
            // Shift the memory block left:
            //  [0] -- [1] -- [2] -- ... -- [n - 2] -- [n - 1]
            //   ^      ^
            //  dst    src
            //  [1] -- [2] -- [3] -- ... -- [n - 1] -- (value)
            std::memmove(this->m_destination_ptr, this->m_source_ptr, this->m_tail_memory_size); // Move the data.
            *this->m_back_ptr = value;
        } // shift_back(...)
    }; // struct sliding_vector

    /** An auxiliary structure to discard old observations. */
    template <std::default_initializable t_value_type, std::size_t t_size>
    struct sliding_array : public std::array<t_value_type, t_size>
    {
        using type = sliding_array<t_value_type, t_size>;
        using base_type = std::array<t_value_type, t_size>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the observed sequence. */
        void shift_back(const value_type& value) noexcept
        {
            constexpr std::size_t tail_size = (t_size - 1);
            std::memmove(this->data(), this->data() + 1, tail_size * sizeof(value_type)); // Move the data.
            this->back() = value;
        } // shift_back(...)
    }; // struct sliding_array

    /** Simple time window with two observations. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 2> : public std::array<t_value_type, 2>
    {
        using type = sliding_array<t_value_type, 2>;
        using base_type = std::array<t_value_type, 2>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the observed sequence. */
        void shift_back(const value_type& value) noexcept
        {
            std::swap(this->front(), this->back());
            this->back() = value;
        } // shift_back(...)
    }; // struct sliding_array

    /** Simple time window with one observation. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 1> : public std::array<t_value_type, 1>
    {
        using type = sliding_array<t_value_type, 1>;
        using base_type = std::array<t_value_type, 1>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the observed sequence. */
        void shift_back(const value_type& value) noexcept
        {
            this->back() = value;
        } // shift_back(...)
    }; // struct sliding_array

    /** Trivial time window. */
    template <std::default_initializable t_value_type>
    struct sliding_array<t_value_type, 0> : public std::array<t_value_type, 0>
    {
        using type = sliding_array<t_value_type, 0>;
        using base_type = std::array<t_value_type, 0>;
        using value_type = t_value_type;

        using base_type::array;

        /** Adds another element to the observed sequence. */
        void shift_back(const value_type& /*value*/) noexcept { }
    }; // struct sliding_array
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_SLIDING_ARRAY_HPP_INCLUDED
