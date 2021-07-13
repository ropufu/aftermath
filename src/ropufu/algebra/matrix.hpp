
#ifndef ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED

#include "matrix_arrangement.hpp"
#include "matrix_index.hpp"
#include "matrix_mask.hpp"
#include "matrix_slice.hpp"
#include "../concepts.hpp"
#include "../simple_vector.hpp"

#include <concepts>  // std::same_as, std::default_initializable, std::equality_comparable
#include <cstddef>   // std::size_t, std::nullptr_t
#include <limits>    // std::numeric_limits
#include <memory>    // std::allocator, std::allocator_traits
#include <ranges>    // std::ranges:range
#include <stdexcept> // std::out_of_range, std::logic_error
#include <type_traits> // std::void_t, std::is_convertible_v
#include <utility>   // std::move, std::swap
#include <vector>    // std::vector

#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(BINOP, OPNAME)                                        \
template <typename t_derived_type, typename t_value_type>                                               \
struct matrix_##OPNAME##_op_module { };                                                                 \
                                                                                                        \
template <typename t_derived_type, ropufu::closed_under_##OPNAME t_value_type>                          \
struct matrix_##OPNAME##_op_module<t_derived_type, t_value_type>                                        \
{                                                                                                       \
private:                                                                                                \
    using matrix_type = t_derived_type;                                                                 \
    using scalar_type = t_value_type;                                                                   \
                                                                                                        \
public:                                                                                                 \
    matrix_type& operator BINOP##=(const matrix_type& other)                                            \
    {                                                                                                   \
        using value_type = scalar_type;                                                                 \
        matrix_type& self = static_cast<matrix_type&>(*this);                                           \
        if (!matrix_type::compatible(self, other)) throw std::logic_error("Matrices incompatible.");    \
                                                                                                        \
        const value_type* right_ptr = other.cbegin();                                                   \
        value_type* left_end_ptr = self.end();                                                          \
        for (value_type* left_ptr = self.begin(); left_ptr != left_end_ptr; ++left_ptr)                 \
        {                                                                                               \
            (*left_ptr) BINOP##= (*right_ptr);                                                          \
            ++right_ptr;                                                                                \
        }                                                                                               \
        return self;                                                                                    \
    }                                                                                                   \
                                                                                                        \
    matrix_type& operator BINOP##=(const scalar_type& other) noexcept                                   \
    {                                                                                                   \
        using value_type = scalar_type;                                                                 \
        matrix_type& self = static_cast<matrix_type&>(*this);                                           \
        value_type* end_ptr = self.end();                                                               \
        for (value_type* it = self.begin(); it != end_ptr; ++it)                                        \
        {                                                                                               \
            (*it) BINOP##= other;                                                                       \
        }                                                                                               \
        return self;                                                                                    \
    }                                                                                                   \
                                                                                                        \
    friend matrix_type operator BINOP(matrix_type left, const matrix_type& right)                       \
    {                                                                                                   \
        left BINOP##= right; return left;                                                               \
    }                                                                                                   \
                                                                                                        \
    friend matrix_type operator BINOP(matrix_type left, const scalar_type& right) noexcept              \
    {                                                                                                   \
        left BINOP##= right; return left;                                                               \
    }                                                                                                   \
};                                                                                                      \

#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(OPNAME) \
matrix_##OPNAME##_op_module<                                      \
    matrix<t_value_type, t_allocator_type, t_arrangement_type>,   \
    t_value_type>                                                 \


#define ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(OPNAME)                 \
template <typename, typename> friend struct detail::matrix_##OPNAME##_op_module; \


namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename, typename, typename, typename = void> struct has_two_dimensional_indexer : public std::false_type { };

        template <typename t_type, typename t_value_type,  typename t_size_type>
        struct has_two_dimensional_indexer<t_type, t_value_type, t_size_type,
            std::void_t<decltype( std::is_convertible_v<t_type, t_value_type(*)(t_size_type, t_size_type)> )>> : public std::true_type { };

        template <typename t_type, typename t_value_type, typename t_size_type>
        inline constexpr bool has_two_dimensional_indexer_v = has_two_dimensional_indexer<t_type, t_value_type, t_size_type>::value;
        
        template <typename t_derived_type, typename t_container_type, typename t_arrangement_type>
        struct matrix_wipe_module
        {
            using derived_type = t_derived_type;
            using container_type = t_container_type;
            using arrangement_type = t_arrangement_type;
            using value_type = typename container_type::value_type;
            using size_type = typename container_type::size_type;

            static constexpr bool is_enabled = false;
        }; // struct matrix_wipe_module

        template <typename t_derived_type, ropufu::wipeable t_container_type, typename t_arrangement_type>
        struct matrix_wipe_module<t_derived_type, t_container_type, t_arrangement_type>
        {
            using derived_type = t_derived_type;
            using container_type = t_container_type;
            using arrangement_type = t_arrangement_type;
            using value_type = typename container_type::value_type;
            using size_type = typename container_type::size_type;

            static constexpr bool is_enabled = true;

            /** @brief Overwrites allocated memory with zeros. */
            void wipe() noexcept
            {
                derived_type* that = static_cast<derived_type*>(this);
                that->m_container.wipe();
            } // wipe(...)

            void make_diagonal(value_type value) noexcept
            {
                derived_type& self = static_cast<derived_type&>(*this);

                size_type m = self.height();
                size_type n = self.width();
                size_type k = (m > n) ? (n) : (m);

                this->wipe();
                for (size_type i = 0; i < k; ++i) self(i, i) = value;
            } // make_diagonal(...)

            /** Creates an uninitialized matrix. */
            static derived_type uninitialized(size_type height, size_type width) noexcept
            {
                derived_type result {};
                result.m_container = container_type::uninitialized(height * width);
                result.m_arrangement = arrangement_type(height, width);
                return result;
            } // uninitialized(...)
        }; // struct matrix_wipe_module<...>
        
        template <typename t_derived_type, typename t_value_type>
        struct matrix_inequality_op_module { };

        template <typename t_derived_type, std::equality_comparable t_value_type>
        struct matrix_inequality_op_module<t_derived_type, t_value_type>
        {
        private:
            using matrix_type = t_derived_type;
            using scalar_type = t_value_type;
        
        public:
            bool operator ==(const matrix_type& other) const noexcept
            {
                const matrix_type& self = static_cast<const matrix_type&>(*this);
                // Check dimensions.
                if (!matrix_type::compatible(self, other)) return false;

                const scalar_type* right_ptr = other.cbegin();
                const scalar_type* left_end_ptr = self.cend();
                for (const scalar_type* left_ptr = self.cbegin(); left_ptr != left_end_ptr; ++left_ptr)
                {
                    if (*(left_ptr) != *(right_ptr)) return false;
                    ++right_ptr;
                } // for (...)
                return true;
            } // operator /=(...)

            bool operator ==(const scalar_type& other) const noexcept
            {
                const matrix_type& self = static_cast<const matrix_type&>(*this);

                const scalar_type* left_end_ptr = self.cend();
                for (const scalar_type* left_ptr = self.cbegin(); left_ptr != left_end_ptr; ++left_ptr)
                {
                    if (*(left_ptr) != other) return false;
                } // for (...)
                return true;
            } // operator /=(...)

            /** Checks two matrices for inequality. */
            bool operator !=(const matrix_type& other) const noexcept
            {
                return !(this->operator ==(other));
            } // operator !=(...)

            /** Checks if all entries of the matrix equal the constant. */
            bool operator !=(const scalar_type& other) const noexcept
            {
                return !(this->operator ==(other));
            } // operator !=(...)
        }; // struct equality_op_module
        
        template <typename t_derived_type, typename t_value_type, typename t_size_type>
        struct matrix_multiplication_module { };

        template <typename t_derived_type, ropufu::arithmetic t_value_type, typename t_size_type>
        struct matrix_multiplication_module<t_derived_type, t_value_type, t_size_type>
        {
        private:
            using matrix_type = t_derived_type;
            using scalar_type = t_value_type;
            using size_type = t_size_type;
        
        public:
            static matrix_type matrix_multiply(const matrix_type& left, const matrix_type& right)
            {
                matrix_type destination(left.height(), right.width());
                matrix_type::matrix_multiply(&destination, left, right);
                return destination;
            } // matrix_multiply(...)

            static void matrix_multiply(matrix_type* destination, const matrix_type& left, const matrix_type& right)
            {
                size_type m = left.height();
                size_type n = right.width();
                size_type k = right.height();

                if (left.width() != k) throw std::logic_error("Matrices incompatible.");
                if (destination->height() != m) throw std::logic_error("Sorage matrix height incompatible.");
                if (destination->width() != n) throw std::logic_error("Sorage matrix width incompatible.");

                for (size_type i = 0; i < m; ++i)
                {
                    for (size_type j = 0; j < n; ++j)
                    {
                        scalar_type& x = destination->operator ()(i, j);
                        x = 0;
                        for (size_type r = 0; r < k; ++r) x += left(i, r) * right(r, j);
                    } // for (...)
                } // for (...)
            } // matrix_multiply(...)
        }; // struct matrix_multiplication_module
        
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(+, addition)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(-, subtraction)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(*, multiplication)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(/, division)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(&, binary_and)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(|, binary_or)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_OP_MODULE(^, binary_xor)
    } // namespace detail

    /** @brief A rectangular array. */
    template <std::default_initializable t_value_type,
        typename t_allocator_type = std::allocator<t_value_type>,
        typename t_arrangement_type = row_major<typename std::allocator_traits<t_allocator_type>::size_type>>
        requires std::same_as<typename std::allocator_traits<t_allocator_type>::size_type, typename t_arrangement_type::size_type>
    struct matrix;

    /** @brief Row major matrix with default allocator. */
    template <std::default_initializable t_value_type>
    using rmatrix_t = matrix<t_value_type,
        std::allocator<t_value_type>,
        row_major<typename std::allocator_traits<std::allocator<t_value_type>>::size_type>>;

    /** @brief Column major matrix with default allocator. */
    template <std::default_initializable t_value_type>
    using cmatrix_t = matrix<t_value_type,
        std::allocator<t_value_type>,
        column_major<typename std::allocator_traits<std::allocator<t_value_type>>::size_type>>;

    /** @brief A rectangular array. */
    template <std::default_initializable t_value_type, typename t_allocator_type, typename t_arrangement_type>
        requires std::same_as<typename std::allocator_traits<t_allocator_type>::size_type, typename t_arrangement_type::size_type>
    struct matrix
        : public detail::matrix_wipe_module<
            matrix<t_value_type, t_allocator_type, t_arrangement_type>,
            simple_vector<t_value_type, t_allocator_type>,
            t_arrangement_type>,
        public detail::matrix_inequality_op_module<
            matrix<t_value_type, t_allocator_type, t_arrangement_type>,
            t_value_type>,
        public detail::matrix_multiplication_module<
            matrix<t_value_type, t_allocator_type, t_arrangement_type>,
            t_value_type,
            typename simple_vector<t_value_type, t_allocator_type>::size_type>,
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(addition),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(subtraction),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(multiplication),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(division),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(binary_and),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(binary_or),
        public detail::ROPUFU_AFTERMATH_ALGEBRA_MATRIX_INHERIT_OP_MODULE(binary_xor)
    {
        using type = matrix<t_value_type, t_allocator_type, t_arrangement_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;
        using arrangement_type = t_arrangement_type;

        template <std::default_initializable t_other_value_type = value_type, typename t_other_allocator_type = allocator_type>
        using container_t = simple_vector<t_other_value_type, t_other_allocator_type>;

        using size_type = typename container_t<>::size_type;
        using signed_size_type = std::make_signed_t<size_type>;
        using index_type = matrix_index<size_type>;
        using mask_type = matrix_mask<std::allocator<bool>, arrangement_type>;
        
        using iterator_type = typename container_t<>::iterator_type;
        using const_iterator_type = typename container_t<>::const_iterator_type;

        template <typename, typename, typename> friend struct detail::matrix_wipe_module;

        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(addition)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(subtraction)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(multiplication)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(division)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(binary_and)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(binary_or)
        ROPUFU_AFTERMATH_ALGEBRA_MATRIX_FRIEND_OP_MODULE(binary_xor)

        template <typename, typename> friend struct detail::matrix_inequality_op_module;

        using wipe_module = detail::matrix_wipe_module<type, container_t<>, arrangement_type>;

    private:
        container_t<> m_container = {}; // Flat data.
        arrangement_type m_arrangement = {}; // Dimensions and structure of the matrix.

        bool is_valid_row_index(size_type index) const noexcept
        {
            if constexpr (std::numeric_limits<size_type>::is_signed)
                if (index < 0) return false;
            if (index >= this->height()) return false;
            return true;
        } // is_valid_row_index(...)

        bool is_valid_column_index(size_type index) const noexcept
        {
            if constexpr (std::numeric_limits<size_type>::is_signed)
                if (index < 0) return false;
            if (index >= this->width()) return false;
            return true;
        } // is_valid_column_index(...)

    public:
        /** @brief Creates a matrix by stealing from \p other. */
        matrix(type&& other) noexcept
            : m_container(std::move(other.m_container)),
            m_arrangement(other.m_arrangement)
        {
        } // matrix(...)

        /** @brief Copies a matrix by stealing from \p other. */
        type& operator =(type&& other) noexcept
        {
            this->m_container = std::move(other.m_container);
            this->m_arrangement = other.m_arrangement;
            return *this;
        } // operator =(...)

        /** @brief Creates an empty matrix. */
        matrix() noexcept { }

        /** @brief Creates an empty (default-constructed) matrix of a given size. */
        matrix(size_type height, size_type width)
            : m_container(height * width), m_arrangement(height, width)
        {
        } // matrix(...)

        /** @brief Creates a matrix with all entries set to \p value. */
        matrix(size_type height, size_type width, const value_type& value)
            : m_container(height * width, value), m_arrangement(height, width)
        {
        } // matrix(...)

        /** @brief Creates a matrix from a list of rows. */
        matrix(std::initializer_list<std::initializer_list<value_type>> values)
        {
            size_type m = values.size();
            size_type n = 0;
            for (const std::initializer_list<value_type>& row : values) if (n < row.size()) n = row.size();
            for (const std::initializer_list<value_type>& row : values) if (n != row.size()) throw std::logic_error("Rows have to be of the same size.");

            this->m_container = container_t<>(m * n);
            this->m_arrangement = arrangement_type(m, n);

            size_type i = 0;
            for (const std::initializer_list<value_type>& row : values)
            {
                size_type j = 0;
                for (const value_type& x : row)
                {
                    this->operator ()(i, j) = x;
                    ++j;
                } // for (...)
                ++i;
            } // for (...)
        } // matrix(...)

        /** @brief Creates a matrix of a given size with values generatred by \c generator: (row_index, column_index) -> value. */
        template <typename t_generator_type>
            requires detail::has_two_dimensional_indexer_v<t_generator_type&&, value_type, size_type>
        static type generate(size_type height, size_type width, t_generator_type&& generator)
        {
            type result { height, width };
            for (size_type i = 0; i < height; ++i)
                for (size_type j = 0; j < width; ++j)
                    result(i, j) = generator(i, j);
            return result;
        } // generate(...)

        /** @brief Creates a matrix from another sequence. */
        template <std::ranges::sized_range t_container_type>
            requires std::same_as<typename t_container_type::value_type, value_type>
        static type column_vector(const t_container_type& container)
        {
            type result {};
            result.m_container = container_t<>(container);
            result.m_arrangement = arrangement_type(container.size(), 1);
            return result;
        } // column_vector(...)

        /** @brief Creates a matrix from another sequence. */
        template <std::ranges::sized_range t_container_type>
            requires std::same_as<typename t_container_type::value_type, value_type>
        static type row_vector(const t_container_type& container)
        {
            type result {};
            result.m_container = container_t<>(container);
            result.m_arrangement = arrangement_type(1, container.size());
            return result;
        } // column_vector(...)

        /** @brief Creates a matrix as a copy of another matrix. */
        /*implicit*/ matrix(const type& other)
            : m_container(other.m_container), m_arrangement(other.m_arrangement)
        {
        } // matrix(...)

        /** @brief Creates a matrix as a copy of another matrix by casting its underlying values. */
        template <typename t_other_value_type, typename t_other_allocator_type>
        explicit operator matrix<t_other_value_type, t_other_allocator_type, arrangement_type>() const
        {
            using other_type = matrix<t_other_value_type, t_other_allocator_type, arrangement_type>;
            other_type other {this->height(), this->width()};
            auto it = this->cbegin();
            for (t_other_value_type& x : other)
            {
                x = static_cast<t_other_value_type>(*(it));
                ++it;
            } // for (...)
            return other;
        } // static_cast<...>

        /** @brief Overwrites the matrix with values from \p other. */
        type& operator =(const type& other) noexcept
        {
            if (this == &other) return *this; // Do nothing if this is self-assignment.

            this->m_container = other.m_container;
            this->m_arrangement = other.m_arrangement;
            return *this;
        } // operator =(...)

        /** @brief Overwrites each entry of the matrix with \p value. */
        type& operator =(const value_type& value) noexcept
        {
            this->fill(value);
            return *this;
        } // operator =(...)

        /** Fills matrix with \p value. */
        void fill(const value_type& value) noexcept
        {
            this->m_container.fill(value);
        } // fill(...)

        /** @brief Transforms every element of the matrix by applying \p action to it.
         *  @param action Has to implement (value_type&) -> void.
         */
        template <typename t_action_type>
            requires ropufu::unary_action<t_action_type, value_type&>
        void transform(t_action_type&& action) noexcept
        {
            for (value_type& x : this->m_container) action(x);
        } // transform(...)

        /** Height of the matrix. */
        size_type height() const noexcept { return this->m_arrangement.height(); }

        /** Width of the matrix. */
        size_type width() const noexcept { return this->m_arrangement.width(); }
                
        /** Number of elements in the matrix. */
        size_type size() const noexcept { return this->m_arrangement.size(); }

        /** Checks if the matrix is empty. */
        bool empty() const noexcept { return this->m_arrangement.empty(); }

        const value_type* data() const noexcept { return this->m_container.data(); }
        value_type* data() noexcept { return this->m_container.data(); }

        /** Checks if the matrix is a square matrix. */
        bool square() const noexcept { return this->m_arrangement.square(); }

        /** Checks if the matrix is lower-triangular. */
        bool lower_triangular() const noexcept
        {
            size_type m = this->height();
            size_type n = this->width();
            size_type s = (m < n) ? (m) : (n);
            
            for (size_type j = 0; j < s; ++j)
                for (size_type i = 0; i < j; ++i)
                    if (this->operator ()(i, j) != 0) return false;

            for (size_type j = s; j < n; ++j)
                for (size_type i = 0; i < m; ++i)
                    if (this->operator ()(i, j) != 0) return false;

            return true;
        } // lower_triangular(...)

        /** Checks if the matrix is upper-triangular. */
        bool upper_triangular() const noexcept
        {
            size_type m = this->height();
            size_type n = this->width();
            size_type s = (m < n) ? (m) : (n);

            for (size_type i = 0; i < s; ++i)
                for (size_type j = 0; j < i; ++j)
                    if (this->operator ()(i, j) != 0) return false;

            for (size_type i = s; i < m; ++i)
                for (size_type j = 0; j < n; ++j)
                    if (this->operator ()(i, j) != 0) return false;

            return true;
        } // upper_triangular(...)

        /** @brief Re-shape the matrix.
         *  @remark The behavior of this operation may depend on \c arrangement_type of this matrix.
         */
        bool try_reshape(size_type height, size_type width) noexcept
        {
            return this->m_arrangement.try_reshape(height, width);
        } // reshape(...)

        bool try_swap_rows(size_type index_a, size_type index_b) noexcept
        {
            if (!this->is_valid_row_index(index_a)) return false;
            if (!this->is_valid_row_index(index_b)) return false;

            if (index_a == index_b) return true;

            auto a = this->row(index_a);
            auto b = this->row(index_b);

            auto it = b.begin();
            for (value_type& x : a)
            {
                std::swap(x, *it);
                ++it;
            } // for (...)

            return true;
        } // try_swap_rows(...)

        bool try_swap_columns(size_type index_a, size_type index_b) noexcept
        {
            if (!this->is_valid_column_index(index_a)) return false;
            if (!this->is_valid_column_index(index_b)) return false;

            if (index_a == index_b) return true;

            auto a = this->column(index_a);
            auto b = this->column(index_b);

            auto it = b.begin();
            for (value_type& x : a)
            {
                std::swap(x, *it);
                ++it;
            } // for (...)

            return true;
        } // try_swap_columns(...)

        /** @brief Create a mask where every element is either
         *  marked (if \p value = true) or unmarked (if \p value = false). */
        mask_type make_mask(bool value = false) const noexcept
        {
            return {this->height(), this->width(), value};
        } // make_mask(...)

        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(size_type row_index, size_type column_index) const noexcept { return this->is_valid_row_index(row_index) && this->is_valid_column_index(column_index); }
        /** @brief Checks if the index is within matrix bounds. */
        bool within_bounds(const index_type& index) const noexcept { return this->is_valid_row_index(index.row) && this->is_valid_column_index(index.column); }

        /** @brief Access matrix elements. No bound checks are performed. */
        const value_type& operator ()(size_type row_index, size_type column_index) const
        {
            return this->m_container[this->m_arrangement.flatten(row_index, column_index)];
        } // operator ()(...)
        /** @brief Access matrix elements. No bound checks are performed. */
        value_type& operator ()(size_type row_index, size_type column_index)
        {
            return this->m_container[this->m_arrangement.flatten(row_index, column_index)];
        } // operator ()(...)
        
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
            if (!this->is_valid_row_index(row_index)) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (!this->is_valid_column_index(column_index)) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        } // at(...)
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(size_type row_index, size_type column_index)
        {
            if (!this->is_valid_row_index(row_index)) throw std::out_of_range("Row index must be less than the height of the matrix.");
            if (!this->is_valid_column_index(column_index)) throw std::out_of_range("Column index must be less than the width of the matrix.");
            return this->operator ()(row_index, column_index);
        } // at(...)
        
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        const value_type& at(const index_type& index) const { return this->at(index.row, index.column); }
        /** @brief Access matrix elements.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        value_type& at(const index_type& index) { return this->at(index.row, index.column); }

        /** Checks whether dimensions of the two matrices are the same. */
        static bool compatible(const type& left, const type& right) noexcept
        {
            return
                (left.height() == right.height()) &&
                (left.width() == right.width());
        } // compatible(...)

        const_iterator_type cbegin() const noexcept { return this->m_container.cbegin(); }
        const_iterator_type cend() const noexcept { return this->m_container.cend(); }

        const_iterator_type begin() const noexcept { return this->m_container.begin(); }
        const_iterator_type end() const noexcept { return this->m_container.end(); }

        iterator_type begin() noexcept { return this->m_container.begin(); }
        iterator_type end() noexcept { return this->m_container.end(); }

        /** @brief Access the elements by mask.
         *  @exception std::logic_error Mask and matrix must have the same dimensions.
         */
        decltype(auto) operator ()(const mask_type& mask) const
        {
            if (this->height() != mask.height() || this->width() != mask.width())
                throw std::logic_error("Matrices incompatible.");
            return mask.slice(this->m_container.data());
        } // operator ()(...)

        /** @brief Access the elements by mask.
         *  @exception std::logic_error Mask and matrix must have the same dimensions.
         */
        decltype(auto) operator ()(const mask_type& mask)
        {
            if (this->height() != mask.height() || this->width() != mask.width())
                throw std::logic_error("Matrices incompatible.");
            return mask.slice(this->m_container.data());
        } // operator ()(...)

        /** @brief Access the elements by mask.
         *  @exception std::logic_error Mask and matrix must have the same dimensions.
         */
        decltype(auto) operator [](const mask_type& mask) const { return this->operator ()(mask); }

        /** @brief Access the elements by mask.
         *  @exception std::logic_error Mask and matrix must have the same dimensions.
         */
        decltype(auto) operator [](const mask_type& mask) { return this->operator ()(mask); }

        /** @brief Access the elements of a particular row.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) row(size_type row_index) const
        {
            return this->m_arrangement.row_slice(row_index, this->m_container.data());
        } // row(...)

        /** @brief Access the elements of a particular row.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) row(size_type row_index)
        {
            return this->m_arrangement.row_slice(row_index, this->m_container.data());
        } // row(...)

        /** @brief Access the elements of a particular column.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) column(size_type column_index) const
        {
            return this->m_arrangement.column_slice(column_index, this->m_container.data());
        } // column(...)

        /** @brief Access the elements of a particular column.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) column(size_type column_index)
        {
            return this->m_arrangement.column_slice(column_index, this->m_container.data());
        } // column(...)

        /** @brief Access the elements of the off-diagonal.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) diag(signed_size_type diagonal_index = 0) const
        {
            return this->m_arrangement.diagonal_slice(diagonal_index, this->m_container.data());
        } // diag(...)

        /** @brief Access the elements of the off-diagonal.
         *  @exception std::out_of_range Index outside the dimensions of the matrix.
         */
        decltype(auto) diag(signed_size_type diagonal_index = 0)
        {
            return this->m_arrangement.diagonal_slice(diagonal_index, this->m_container.data());
        } // diag(...)
    }; // struct matrix
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_MATRIX_HPP_INCLUDED
