
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_V2_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_V2_HPP_INCLUDED

#include "matrix_arrangement.hpp"
#include "matrix_index.hpp"
#include "matrix.hpp" // detail::matrix_arrangement

#include <cstddef>   // std::size_t
#include <cstring>   // std::memset
#include <memory>    // std::allocator
#include <stdexcept> // std::out_of_range
#include <string>    // std::string, std::to_string
#include <type_traits> // std::true_type, std::false_type, std::void_t
#include <utility>   // std::forward, std::declval
#include <vector>    // std::vector

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename, typename = void> struct has_add_assign : public std::false_type { };
        template <typename, typename = void> struct has_subtract_assign : public std::false_type { };
        template <typename, typename = void> struct has_multiply_assign : public std::false_type { };
        template <typename, typename = void> struct has_divide_assign : public std::false_type { };

        template <typename t_type> struct has_add_assign<t_type, std::void_t<decltype(std::declval<t_type&>() += std::declval<const t_type&>())>> : public std::true_type { };
        template <typename t_type> struct has_subtract_assign<t_type, std::void_t<decltype(std::declval<t_type&>() -= std::declval<const t_type&>())>> : public std::true_type { };
        template <typename t_type> struct has_multiply_assign<t_type, std::void_t<decltype(std::declval<t_type&>() *= std::declval<const t_type&>())>> : public std::true_type { };
        template <typename t_type> struct has_divide_assign<t_type, std::void_t<decltype(std::declval<t_type&>() /= std::declval<const t_type&>())>> : public std::true_type { };

        template <typename t_type> inline constexpr bool has_add_assign_v = has_add_assign<t_type>::value;
        template <typename t_type> inline constexpr bool has_subtract_assign_v = has_subtract_assign<t_type>::value;
        template <typename t_type> inline constexpr bool has_multiply_assign_v = has_multiply_assign<t_type>::value;
        template <typename t_type> inline constexpr bool has_divide_assign_v = has_divide_assign<t_type>::value;
        
        template <bool t_is_enabled, typename t_derived_type>
        struct add_assign_op_module { };

        template <typename t_derived_type>
        struct add_assign_op_module<true, t_derived_type>
        {
            t_derived_type& operator +=(const t_derived_type& other)
            {
                t_derived_type& self = static_cast<t_derived_type&>(*this);
                if (!t_derived_type::compatible(self, other)) throw std::logic_error("Matrices incompatible.");
                std::size_t count = self.m_values.size();
                for (std::size_t k = 0; k < count; ++k) self.m_values[k] += other.m_values[k];
                return self;
            } // operator +=(...)
            
            /** Note that \p left is passed by value. */
            friend t_derived_type operator +(t_derived_type left, const t_derived_type& right) noexcept { left += right; return left; }
        }; // struct add_assign_op_module
        
        template <bool t_is_enabled, typename t_derived_type>
        struct subtract_assign_op_module { };

        template <typename t_derived_type>
        struct subtract_assign_op_module<true, t_derived_type>
        {
            t_derived_type& operator -=(const t_derived_type& other)
            {
                t_derived_type& self = static_cast<t_derived_type&>(*this);
                if (!t_derived_type::compatible(self, other)) throw std::logic_error("Matrices incompatible.");
                std::size_t count = self.m_values.size();
                for (std::size_t k = 0; k < count; ++k) self.m_values[k] -= other.m_values[k];
                return self;
            } // operator -=(...)
            
            /** Note that \p left is passed by value. */
            friend t_derived_type operator -(t_derived_type left, const t_derived_type& right) noexcept { left -= right; return left; }
        }; // struct subtract_assign_op_module
        
        template <bool t_is_enabled, typename t_derived_type>
        struct multiply_assign_op_module { };

        template <typename t_derived_type>
        struct multiply_assign_op_module<true, t_derived_type>
        {
            t_derived_type& operator *=(const t_derived_type& other)
            {
                t_derived_type& self = static_cast<t_derived_type&>(*this);
                if (!t_derived_type::compatible(self, other)) throw std::logic_error("Matrices incompatible.");
                std::size_t count = self.m_values.size();
                for (std::size_t k = 0; k < count; ++k) self.m_values[k] *= other.m_values[k];
                return self;
            } // operator *=(...)
            
            /** Note that \p left is passed by value. */
            friend t_derived_type operator *(t_derived_type left, const t_derived_type& right) noexcept { left *= right; return left; }
        }; // struct multiply_assign_op_module
        
        template <bool t_is_enabled, typename t_derived_type>
        struct divide_assign_op_module { };

        template <typename t_derived_type>
        struct divide_assign_op_module<true, t_derived_type>
        {
            t_derived_type& operator /=(const t_derived_type& other)
            {
                t_derived_type& self = static_cast<t_derived_type&>(*this);
                if (!t_derived_type::compatible(self, other)) throw std::logic_error("Matrices incompatible.");
                std::size_t count = self.m_values.size();
                for (std::size_t k = 0; k < count; ++k) self.m_values[k] /= other.m_values[k];
                return self;
            } // operator /=(...)
            
            /** Note that \p left is passed by value. */
            friend t_derived_type operator /(t_derived_type left, const t_derived_type& right) noexcept { left /= right; return left; }
        }; // struct divide_assign_op_module
    } // namespace detail

    /** @brief A rectangular array. */
    template <typename t_value_type, bool t_is_row_major = true, typename t_allocator_type = std::allocator<t_value_type>>
    struct matrix_v2;

    /** @brief A rectangular array. */
    template <typename t_value_type, bool t_is_row_major, typename t_allocator_type>
    struct matrix_v2
        : public detail::add_assign_op_module<detail::has_add_assign_v<t_value_type>, matrix_v2<t_value_type, t_is_row_major, t_allocator_type>>,
        public detail::subtract_assign_op_module<detail::has_subtract_assign_v<t_value_type>, matrix_v2<t_value_type, t_is_row_major, t_allocator_type>>,
        public detail::multiply_assign_op_module<detail::has_multiply_assign_v<t_value_type>, matrix_v2<t_value_type, t_is_row_major, t_allocator_type>>,
        public detail::divide_assign_op_module<detail::has_divide_assign_v<t_value_type>, matrix_v2<t_value_type, t_is_row_major, t_allocator_type>>
    {
        static constexpr bool is_row_major = t_is_row_major;
        static constexpr bool is_column_major = !t_is_row_major;

        using type = matrix_v2<t_value_type, t_is_row_major, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

        template <typename, bool, typename> friend struct matrix_v2;
        template <bool, typename> friend struct detail::add_assign_op_module;
        template <bool, typename> friend struct detail::subtract_assign_op_module;
        template <bool, typename> friend struct detail::multiply_assign_op_module;
        template <bool, typename> friend struct detail::divide_assign_op_module;

    private:
        using arrangement_noexcept_type = typename detail::matrix_arrangement<is_row_major, true>;
        using arrangement_checked_type = typename detail::matrix_arrangement<is_row_major, false>;
        using container_type = std::vector<value_type, allocator_type>;
                
        template <typename t_other_value_type>
        using other_t = matrix_v2<t_other_value_type, t_is_row_major, t_allocator_type>;

        std::size_t m_height = 0; // Height of the matrix.
        std::size_t m_width = 0;  // Width of the matrix.
        std::size_t m_size = 0;   // Number of elements in the matrix.
        container_type m_values = {};

    public:
        /** Creates an empty matrix. */
        matrix_v2() noexcept { }

        /** @brief Creates an empty (default-valued) matrix of a given size. */
        matrix_v2(std::size_t height, std::size_t width) noexcept
            : m_height(height), m_width(width), m_size(height * width), m_values(height * width)
        {
            this->m_values.shrink_to_fit();
        } // matrix(...)

        /** Creates a matrix as a copy. */
        matrix_v2(const type& other) noexcept
            : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size), m_values(other.m_values)
        {
            this->m_values.shrink_to_fit();
        } // matrix(...)

        /** Creates a matrix by stealing from \p other. */
        matrix_v2(type&& other) noexcept
            : m_height(other.m_height), m_width(other.m_width), m_size(other.m_size), m_values(std::forward<container_type>(other.m_values))
        {
            this->m_values.shrink_to_fit();
        } // matrix(...)

        /** Creates a matrix from a vector. */
        matrix_v2(const std::vector<value_type>& values, bool is_column_vector = true) noexcept
            : m_height(is_column_vector ? values.size() : 1), m_width(is_column_vector ? 1 : values.size()), m_values(values)
        {
            this->m_values.shrink_to_fit();
        } // matrix(...)

        /** Creates a matrix from a vector. */
        matrix_v2(std::vector<value_type>&& values, bool is_column_vector = true) noexcept
            : m_height(is_column_vector ? values.size() : 1), m_width(is_column_vector ? 1 : values.size()), m_values(std::forward<container_type>(values))
        {
            this->m_values.shrink_to_fit();
        } // matrix(...)

        /** @brief Creates a matrix of a given size with values generatred by \c generator: (i, j) -> value_type. */
        template <typename t_generator_type>
        matrix_v2(std::size_t height, std::size_t width, t_generator_type&& generator) noexcept
            : matrix_v2(height, width)
        {
            for (std::size_t i = 0; i < height; ++i)
                for (std::size_t j = 0; j < width; ++j)
                    this->m_values[arrangement_noexcept_type::flatten(i, j, height, width)] = generator(i, j);
        } // matrix(...)

        /** Creates a matrix with all entries set to \p value. */
        matrix_v2(std::size_t height, std::size_t width, const value_type& value) noexcept
            : matrix_v2(height, width)
        {
            this->fill(value);
        } // matrix(...)

        /** Copies a matrix. */
        type& operator =(const type& other) noexcept
        {
            if (this != &other)
            {
                this->m_height = other.m_height;
                this->m_width = other.m_width;
                this->m_size = other.m_size;
                this->m_values = other.m_values;
            } // if (...)
            return *this;
        } // operator =(...)

        /** Copies a matrix by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->m_height = other.m_height;
            this->m_width = other.m_width;
            this->m_size = other.m_size;
            this->m_values = std::forward<container_type>(other.m_values);
            return *this;
        } // operator =(...)

        /** @brief Overwrites allocated memory with zeros. */
        void wipe() noexcept
        {
            std::memset(this->m_values.data(), 0, this->m_size * sizeof(value_type));
        } // wipe(...)

        /** Fills matrix with \p value. */
        void fill(const value_type& value) noexcept
        {
            for (value_type& x : this->m_values) x = value;
        } // fill(...)

        /** Transforms every element of the matrix by applying \p action to it. */
        template <typename t_action_type>
        void transform(t_action_type&& action) noexcept
        {
            for (value_type& x : this->m_values) x = action(value);
        } // transform(...)

        /** Height of the matrix. */
        std::size_t height() const noexcept { return this->m_height; }

        /** Width of the matrix. */
        std::size_t width() const noexcept { return this->m_width; }
                
        /** Number of elements in the matrix. */
        std::size_t size() const noexcept { return this->m_size; }

        /** Checks if the matrix is empty. */
        bool empty() const noexcept { return this->m_size == 0; }

        /** @brief Re-shape the matrix.
         *  @exception not_an_error::logic_error This error is pushed to \c quiet_error if new size does not match old size.
         *  @remark The behavior of this operation depends on whether this is a row- or column-major matrix.
         */
        bool try_reshape(std::size_t height, std::size_t width) noexcept
        {
            if (height * width != this->m_size) return false;
            this->m_height = height;
            this->m_width = width;
        } // reshape(...)
                
        /** Access the first matrix element. */
        const value_type& front() const noexcept { return this->m_values.front(); }
        /** Access the first matrix element. */
        value_type& front() noexcept { return this->m_values.front(); }
                
        /** Access the last matrix element. */
        const value_type& back() const noexcept { return this->m_values.back(); }
        /** Access the last matrix element. */
        value_type& back() noexcept { return this->m_values.back(); }

        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(std::size_t row_index, std::size_t column_index) const noexcept { return row_index < this->m_height && column_index < this->m_width; }
        
        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(const matrix_index& index) const noexcept { return index.row < this->m_height && index.column < this->m_width; }

        /** @brief Access matrix elements. */
        const value_type& at(std::size_t row_index, std::size_t column_index) const { return this->m_values.at(arrangement_checked_type::flatten(row_index, column_index, this->m_height, this->m_width)); }
        /** @brief Access matrix elements. */
        value_type& at(std::size_t row_index, std::size_t column_index) { return this->m_values.at(arrangement_checked_type::flatten(row_index, column_index, this->m_height, this->m_width)); }
        
        /** @brief Access matrix elements. */
        const value_type& at(const matrix_index& index) const { return this->at(index.row, index.column); }
        /** @brief Access matrix elements. */
        value_type& at(const matrix_index& index) { return this->at(index.row, index.column); }

        /** @brief Access matrix elements. */
        const value_type& operator ()(std::size_t row_index, std::size_t column_index) const { return this->m_values[arrangement_noexcept_type::flatten(row_index, column_index, this->m_height, this->m_width)]; }
        /** @brief Access matrix elements. */
        value_type& operator ()(std::size_t row_index, std::size_t column_index) { return this->m_values[arrangement_noexcept_type::flatten(row_index, column_index, this->m_height, this->m_width)]; }
        
        /** @brief Access matrix elements. */
        const value_type& operator ()(const matrix_index& index) const { return this->operator ()(index.row, index.column); }
        /** @brief Access matrix elements. */
        value_type& operator ()(const matrix_index& index) { return this->operator ()(index.row, index.column); }

        /** @brief Access matrix elements. */
        const value_type& operator [](const matrix_index& index) const { return this->operator ()(index.row, index.column); }
        /** @brief Access matrix elements. */
        value_type& operator [](const matrix_index& index) { return this->operator ()(index.row, index.column); }

        /** Checks two matrices for equality. */
        bool operator ==(const type& other) const noexcept
        {
            return
                (this->m_height == other.m_height) &&
                (this->m_width == other.m_width) &&
                (this->m_values == other.m_values);
        } // operator ==(...)

        /** Checks two matrices for inequality. */
        bool operator !=(const type& other) const noexcept
        {
            return !(this->operator ==(other));
        } // operator !=(...)

        static bool compatible(const type& left, const type& right) noexcept { return (left.m_height == right.m_height) && (left.m_width == right.m_width); }

        auto cbegin() const noexcept { return this->m_values.cbegin(); }
        auto cend() const noexcept { return this->m_values.cend(); }

        auto begin() const noexcept { return this->m_values.begin(); }
        auto end() const noexcept { return this->m_values.end(); }

        auto begin() noexcept { return this->m_values.begin(); }
        auto end() noexcept { return this->m_values.end(); }
    }; // struct matrix_v2
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_V2_HPP_INCLUDED
