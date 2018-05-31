
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED

#include "matrix_arrangement.hpp"
#include "matrix_index.hpp"

#include <array>     // std::array
#include <cstddef>   // std::size_t, std::nullptr_t
#include <cstring>   // std::memset, std::memcpy
#include <initializer_list> // std::initializer_list
#include <memory>    // std::allocator, std::allocator_traits
#include <stdexcept> // std::out_of_range, std::logic_error
#include <string>    // std::string, std::to_string
#include <type_traits> // ...
#include <utility>   // std::forward, std::declval
#include <vector>    // std::vector

#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(BINOP, OPNAME)                                        \
template <typename, typename = void> struct has_##OPNAME##_assign : public std::false_type { };         \
                                                                                                        \
template <typename t_type> struct has_##OPNAME##_assign<                                                \
    t_type, std::void_t<decltype(std::declval<t_type&>() BINOP= std::declval<const t_type&>())>>        \
    : public std::true_type { };                                                                        \
                                                                                                        \
template <typename t_type>                                                                              \
inline constexpr bool has_##OPNAME##_assign_v = has_##OPNAME##_assign<t_type>::value;                   \
                                                                                                        \
template <bool t_is_enabled, typename t_derived_type>                                                   \
struct OPNAME##_assign_op_module { };                                                                   \
                                                                                                        \
template <typename t_derived_type>                                                                      \
struct OPNAME##_assign_op_module<true, t_derived_type>                                                  \
{                                                                                                       \
    t_derived_type& operator BINOP=(const t_derived_type& other)                                        \
    {                                                                                                   \
        using size_type = typename t_derived_type::size_type;                                           \
        using value_type = typename t_derived_type::value_type;                                         \
        t_derived_type& self = static_cast<t_derived_type&>(*this);                                     \
        if (!t_derived_type::compatible(self, other)) throw std::logic_error("Matrices incompatible."); \
        value_type* left_ptr = self.m_begin_ptr;                                                        \
        const value_type* right_ptr = other.m_begin_ptr;                                                \
        for (size_type k = 0; k < self.m_size; ++k)                                                     \
        {                                                                                               \
            (*left_ptr) BINOP= (*right_ptr);                                                            \
            ++left_ptr;                                                                                 \
            ++right_ptr;                                                                                \
        }                                                                                               \
        return self;                                                                                    \
    }                                                                                                   \
                                                                                                        \
    friend t_derived_type operator BINOP(t_derived_type left, const t_derived_type& right) noexcept     \
    {                                                                                                   \
        left BINOP= right; return left;                                                                 \
    }                                                                                                   \
};                                                                                                      \


namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename, typename = void> struct has_inequality : public std::false_type { };
        template <typename t_type> struct has_inequality<t_type, std::void_t<decltype(std::declval<const t_type&>() != std::declval<const t_type&>())>> : public std::true_type { };
        template <typename t_type> inline constexpr bool has_inequality_v = has_inequality<t_type>::value;
        
        template <bool t_is_enabled, typename t_derived_type, typename t_value_type, typename t_size_type>
        struct wipe_module
        {
            wipe_module() noexcept { }
        }; // struct wipe_module

        template <typename t_derived_type, typename t_value_type, typename t_size_type>
        struct wipe_module<true, t_derived_type, t_value_type, t_size_type>
        {
            using derived_type = typename t_derived_type;
            using value_type = typename t_value_type;
            using size_type = typename t_size_type;

            /** Creates an uninitialized matrix. */
            static derived_type uninitialized(size_type height, size_type width) { return derived_type(nullptr, height, width); }

            /** @brief Overwrites allocated memory with zeros.
             *  @warning No destructors are called.
             */
            void wipe() noexcept
            {
                derived_type& self = static_cast<derived_type&>(*this);
                std::memset(self.m_begin_ptr, 0, static_cast<std::size_t>(self.m_size) * sizeof(value_type));
            } // wipe(...)
        }; // struct wipe_module<...>
        
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(+, add)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(-, subtract)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(*, multiply)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(/, divide)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(|, binor)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(&, binand)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(^, binxor)
        
        template <bool t_is_enabled, typename t_derived_type>
        struct inequality_op_module { };

        template <typename t_derived_type>
        struct inequality_op_module<true, t_derived_type>
        {
            bool operator ==(const t_derived_type& other) const noexcept
            {
                using size_type = typename t_derived_type::size_type;
                using value_type = typename t_derived_type::value_type;
                const t_derived_type& self = static_cast<const t_derived_type&>(*this);
                // Check dimensions.
                if (!t_derived_type::compatible(self, other)) return false;
                // Check values.
                const value_type* left_ptr = self.m_begin_ptr;
                const value_type* right_ptr = other.m_begin_ptr;
                for (size_type k = 0; k < self.m_size; ++k)
                {
                    if (*(left_ptr) != *(right_ptr)) return false;
                    ++left_ptr;
                    ++right_ptr;
                } // for (...)
                return true;
            } // operator /=(...)

            /** Checks two matrices for inequality. */
            bool operator !=(const t_derived_type& other) const noexcept
            {
                return !(this->operator ==(other));
            } // operator !=(...)
        }; // struct equality_op_module
    } // namespace detail

    /** @brief A rectangular array. */
    template <typename t_value_type, typename t_allocator_type = std::allocator<t_value_type>, typename t_size_type = typename std::allocator_traits<t_allocator_type>::size_type, typename t_arrangement_type = detail::row_major<t_size_type>>
    struct matrix;

    template <typename t_value_type>
    using matrix_row_major = matrix<t_value_type, std::allocator<t_value_type>, typename std::allocator_traits<std::allocator<t_value_type>>::size_type, detail::row_major<typename std::allocator_traits<std::allocator<t_value_type>>::size_type>>;

    template <typename t_value_type>
    using matrix_column_major = matrix<t_value_type, std::allocator<t_value_type>, typename std::allocator_traits<std::allocator<t_value_type>>::size_type, detail::column_major<typename std::allocator_traits<std::allocator<t_value_type>>::size_type>>;

    /** @brief A rectangular array. */
    template <typename t_value_type, typename t_allocator_type, typename t_size_type, typename t_arrangement_type>
    struct matrix
        : public detail::wipe_module<std::is_arithmetic_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>, t_value_type, t_size_type>,
        public detail::add_assign_op_module<detail::has_add_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::subtract_assign_op_module<detail::has_subtract_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::multiply_assign_op_module<detail::has_multiply_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::divide_assign_op_module<detail::has_divide_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::binor_assign_op_module<detail::has_binor_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::binand_assign_op_module<detail::has_binand_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::binxor_assign_op_module<detail::has_binxor_assign_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>,
        public detail::inequality_op_module<detail::has_inequality_v<t_value_type>, matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>>
    {
        using type = matrix<t_value_type, t_allocator_type, t_size_type, t_arrangement_type>;
        using value_type = t_value_type;
        using arrangement_type = t_arrangement_type;
        using allocator_type = t_allocator_type;
        using allocator_traits_type = std::allocator_traits<t_allocator_type>;
        using size_type = t_size_type;
        using index_type = matrix_index<t_size_type>;
        using iterator_type = t_value_type*;
        using const_iterator_type = const t_value_type*;

        static constexpr bool is_trivial = std::is_trivially_constructible_v<t_value_type> && std::is_trivially_destructible_v<t_value_type>;

        template <typename, typename, typename, typename> friend struct matrix;
        template <bool, typename, typename, typename> friend struct detail::wipe_module;
        template <bool, typename> friend struct detail::add_assign_op_module;
        template <bool, typename> friend struct detail::subtract_assign_op_module;
        template <bool, typename> friend struct detail::multiply_assign_op_module;
        template <bool, typename> friend struct detail::divide_assign_op_module;
        template <bool, typename> friend struct detail::binor_assign_op_module;
        template <bool, typename> friend struct detail::binand_assign_op_module;
        template <bool, typename> friend struct detail::binxor_assign_op_module;
        template <bool, typename> friend struct detail::inequality_op_module;

    private:
        template <typename t_other_value_type>
        using other_t = matrix<t_other_value_type, t_allocator_type, t_size_type, t_arrangement_type>;

        size_type m_height = 0; // Height of the matrix.
        size_type m_width = 0;  // Width of the matrix.
        size_type m_size = 0;   // Number of elements in the matrix.
        allocator_type m_allocator = {};
        value_type* m_begin_ptr = nullptr; // Pointer to the first element of the matrix.
        value_type* m_end_ptr = nullptr;   // Pointer to the past-the-last element of the matrix.
        value_type* m_back_ptr = nullptr;  // Pointer to the last element of the matrix.

        /** @brief Creates an uninitialized matrix.
         *  @exception std::bad_alloc Allocation failed.
         */
        matrix(std::nullptr_t, size_type height, size_type width)
            : m_height(height), m_width(width), m_size(height * width)
        {
            static_assert(std::is_default_constructible_v<value_type>, "value_type has to be default constructible.");
            this->allocate();
        } // matrix(...)

        /** @brief Allocates memory for storing matrix elements.
         *  @exception std::bad_alloc Allocation failed.
         *  @exception std::logic_error Deallocation has to be called first.
         */
        void allocate()
        {
            if (this->m_begin_ptr != nullptr) throw std::logic_error("Memory has to be deallocated prior to another allocation call.");
            // Allocate one extra element to make sure de-referencing does not result in exceptions.
            this->m_begin_ptr = this->m_allocator.allocate(static_cast<typename allocator_traits_type::size_type>(this->m_size + 1));
            this->m_end_ptr = (this->m_begin_ptr + this->m_size);
            this->m_back_ptr = (this->m_size == 0) ? (this->m_end_ptr) : (this->m_end_ptr - 1);
            allocator_traits_type::construct(this->m_allocator, this->m_end_ptr); // Construct the out-of-range element.
        } // allocate(...)

        /** @brief Relinquish allocated memory. */
        void deallocate() noexcept
        {
            if (this->m_begin_ptr == nullptr) return; // Check if this matrix has already been deallocated.
            
            if constexpr (!type::is_trivial)
            {
                for (value_type* it = this->m_begin_ptr; it != this->m_end_ptr; ++it) allocator_traits_type::destroy(this->m_allocator, it);
            } // if constexpr (...)
            allocator_traits_type::destroy(this->m_allocator, this->m_end_ptr);

            this->m_allocator.deallocate(this->m_begin_ptr, this->m_size + 1);
            this->m_begin_ptr = nullptr;
            this->m_end_ptr = nullptr;
            this->m_back_ptr = nullptr;
        } // deallocate(...)

    public:
        /** @brief Creates a matrix by stealing from \p other. */
        matrix(type&& other) noexcept
            : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size), 
            m_allocator(std::forward<allocator_type>(other.m_allocator)), m_begin_ptr(other.m_begin_ptr), m_end_ptr(other.m_end_ptr), m_back_ptr(other.m_back_ptr)
        {
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
            other.m_end_ptr = nullptr;
            other.m_back_ptr = nullptr;
        } // matrix(...)

        /** @brief Copies a matrix by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->deallocate(); // Relinquish existing memory.
            // Reset current fields.
            this->m_height = other.m_height;
            this->m_width = other.m_width;
            this->m_size = other.m_size;
            this->m_allocator = std::forward<allocator_type>(other.m_allocator);
            this->m_begin_ptr = other.m_begin_ptr;
            this->m_end_ptr = other.m_end_ptr;
            this->m_back_ptr = other.m_back_ptr;
            // Clear stolen references.
            other.m_allocator = {};
            other.m_begin_ptr = nullptr;
            other.m_end_ptr = nullptr;
            other.m_back_ptr = nullptr;
            return *this;
        } // operator =(...)

        ~matrix() noexcept { this->deallocate(); }

        /** @brief Creates an empty matrix. */
        matrix() : matrix(nullptr, 0, 0) { }

        /** @brief Creates an empty (default-constructed) matrix of a given size. */
        matrix(size_type height, size_type width) : matrix(nullptr, height, width)
        {
            if constexpr (std::is_arithmetic_v<value_type>) std::memset(this->m_begin_ptr, 0, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                for (value_type* it = this->m_begin_ptr; it != this->m_end_ptr; ++it) allocator_traits_type::construct(this->m_allocator, it);
            } // if constexpr (...)
        } // matrix(...)

        /** @brief Creates a matrix with all entries set to \p value. */
        matrix(size_type height, size_type width, const value_type& value) : matrix(nullptr, height, width)
        {
            for (value_type* it = this->m_begin_ptr; it != this->m_end_ptr; ++it) allocator_traits_type::construct(this->m_allocator, it, value);
        } // matrix(...)

        /** @brief Creates a matrix of a given size with values generatred by \c generator: (i, j) -> value_type. */
        template <typename t_generator_type>
        static type generate(size_type height, size_type width, t_generator_type&& generator)
        {
            type result(nullptr, height, width);
            for (size_type i = 0; i < height; ++i)
                for (size_type j = 0; j < width; ++j)
                    allocator_traits_type::construct(result.m_allocator, result.m_begin_ptr + arrangement_type::flatten(i, j, height, width), generator(i, j));
            return result;
        } // matrix(...)

        /** @brief Creates a matrix from a vector. */
        matrix(const std::vector<value_type>& values, bool is_column_vector = true)
            : matrix(nullptr, (is_column_vector ? values.size() : 1), (is_column_vector ? 1 : values.size()))
        {
            if constexpr (type::is_trivial) std::memcpy(this->m_begin_ptr, values.data(), values.size() * sizeof(value_type));
            else
            {
                value_type* it = this->m_begin_ptr;
                for (const value_type& x : values)
                {
                    allocator_traits_type::construct(this->m_allocator, it, x);
                    ++it;
                } // for (...)
            } // if constexpr (...)
        } // matrix(...)

        /** @brief Creates a matrix from an array. */
        template <std::size_t t_vector_size>
        matrix(const std::array<value_type, t_vector_size>& values, bool is_column_vector = true)
            : matrix(nullptr, static_cast<size_type>(is_column_vector ? values.size() : 1), static_cast<size_type>(is_column_vector ? 1 : values.size()))
        {
            if constexpr (type::is_trivial) std::memcpy(this->m_begin_ptr, values.data(), values.size() * sizeof(value_type));
            else
            {
                value_type* it = this->m_begin_ptr;
                for (const value_type& x : values)
                {
                    allocator_traits_type::construct(this->m_allocator, it, x);
                    ++it;
                } // for (...)
            } // if constexpr (...)
        } // matrix(...)

        /** @brief Creates a matrix from an initializer list. */
        matrix(std::initializer_list<value_type>& values, bool is_column_vector = true)
            : matrix(nullptr, static_cast<size_type>(is_column_vector ? values.size() : 1), static_cast<size_type>(is_column_vector ? 1 : values.size()))
        {
            if constexpr (type::is_trivial) std::memcpy(this->m_begin_ptr, values.begin(), values.size() * sizeof(value_type));
            else
            {
                value_type* it = this->m_begin_ptr;
                for (const value_type& x : values)
                {
                    allocator_traits_type::construct(this->m_allocator, it, x);
                    ++it;
                } // for (...)
            } // if constexpr (...)
        } // matrix(...)

        /** @brief Creates a matrix as a copy of another matrix. */
        matrix(const type& other) : matrix(nullptr, other.m_height, other.m_width)
        {
            if constexpr (type::is_trivial) std::memcpy(this->m_begin_ptr, other.m_begin_ptr, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                value_type* left_ptr = this->m_begin_ptr;
                const value_type* right_ptr = other.m_begin_ptr;
                for (size_type k = 0; k < this->m_size; ++k)
                {
                    allocator_traits_type::construct(this->m_allocator, left_ptr, (*right_ptr));
                    ++left_ptr;
                    ++right_ptr;
                } // for (...)
            } // if constexpr (...)
        } // matrix(...)

        /** Overwrites the matrix with values from \p other. */
        type& operator =(const type& other) noexcept
        {
            if (this == &other) return *this; // Do nothing if this is self-assignment.

            // Re-allocate memory if matrices are incompatible.
            if (!type::compatible(*this, other))
            {
                this->deallocate(); // Relinquish existing memory.
                this->m_height = other.m_height;
                this->m_width = other.m_width;
                this->m_size = other.m_size;
                this->allocate(); // Allocate memory to hold new values.
            } // if (...)

            if constexpr (type::is_trivial) std::memcpy(this->m_begin_ptr, other.m_begin_ptr, static_cast<std::size_t>(this->m_size) * sizeof(value_type));
            else
            {
                value_type* left_ptr = this->m_begin_ptr;
                const value_type* right_ptr = other.m_begin_ptr;
                for (size_type k = 0; k < this->m_size; ++k)
                {
                    (*left_ptr) = (*right_ptr);
                    ++left_ptr;
                    ++right_ptr;
                } // for (...)
                return self;
            } // if constexpr (...)

            return *this;
        } // operator =(...)

        /** Fills matrix with \p value. */
        void fill(const value_type& value) noexcept
        {
            for (value_type* it = this->m_begin_ptr; it != this->m_end_ptr; ++it) (*it) = value;
        } // fill(...)

        /** Transforms every element of the matrix by applying \p action : (value_type&) -> void to it. */
        template <typename t_action_type>
        void transform(t_action_type&& action) noexcept
        {
            for (value_type* it = this->m_begin_ptr; it != this->m_end_ptr; ++it) action(*it);
        } // transform(...)

        /** Height of the matrix. */
        size_type height() const noexcept { return this->m_height; }

        /** Width of the matrix. */
        size_type width() const noexcept { return this->m_width; }
                
        /** Number of elements in the matrix. */
        size_type size() const noexcept { return this->m_size; }

        /** Checks if the matrix is empty. */
        bool empty() const noexcept { return this->m_size == 0; }

        /** @brief Re-shape the matrix.
         *  @remark The behavior of this operation depends on \c arrangement_type of this matrix.
         */
        bool try_reshape(size_type height, size_type width) noexcept
        {
            if (height * width != this->m_size) return false;
            this->m_height = height;
            this->m_width = width;
            return true;
        } // reshape(...)
                
        /** Access the first matrix element. */
        const value_type& front() const noexcept { return *(this->m_begin_ptr); }
        /** Access the first matrix element. */
        value_type& front() noexcept { return *(this->m_begin_ptr); }
                
        /** Access the last matrix element. */
        const value_type& back() const noexcept { return *(this->m_back_ptr); }
        /** Access the last matrix element. */
        value_type& back() noexcept { return *(this->m_back_ptr); }

        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(size_type row_index, size_type column_index) const noexcept { return row_index < this->m_height && column_index < this->m_width; }
        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(const index_type& index) const noexcept { return index.row < this->m_height && index.column < this->m_width; }

        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator ()(std::size_t row_index, std::size_t column_index) const { return this->m_begin_ptr[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)]; }
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator ()(std::size_t row_index, std::size_t column_index) { return this->m_begin_ptr[arrangement_type::flatten(row_index, column_index, this->m_height, this->m_width)]; }
        
        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator ()(const index_type& index) const { return this->operator ()(index.row, index.column); }
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator ()(const index_type& index) { return this->operator ()(index.row, index.column); }

        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator [](const index_type& index) const { return this->operator ()(index.row, index.column); }
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator [](const index_type& index) { return this->operator ()(index.row, index.column); }

        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        const value_type& at(size_type row_index, size_type column_index) const
        {
            if (row_index >= this->m_height) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (column_index >= this->m_width) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        }
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(std::size_t row_index, std::size_t column_index)
        {
            if (row_index >= this->m_height) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (column_index >= this->m_width) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        }
        
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        const value_type& at(const index_type& index) const { return this->at(index.row, index.column); }
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(const index_type& index) { return this->at(index.row, index.column); }

        /** Checks whether dimensions of the two matrices are the same. */
        static bool compatible(const type& left, const type& right) noexcept { return (left.m_height == right.m_height) && (left.m_width == right.m_width); }

        const_iterator_type cbegin() const noexcept { return this->m_begin_ptr; }
        const_iterator_type cend() const noexcept { return this->m_end_ptr; }

        const_iterator_type begin() const noexcept { return this->m_begin_ptr; }
        const_iterator_type end() const noexcept { return this->m_end_ptr; }

        iterator_type begin() noexcept { return this->m_begin_ptr; }
        iterator_type end() noexcept { return this->m_end_ptr; }
    }; // struct matrix
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
