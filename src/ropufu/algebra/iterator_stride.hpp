
#ifndef ROPUFU_AFTERMATH_ALGEBRA_ITERATOR_STRIDE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_ITERATOR_STRIDE_HPP_INCLUDED

#include <cstddef> // std::size_t, std::ptrdiff_t
#include <utility> // std::move
#include <vector>  // std::vector

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        /** Fixed step stride. */
        struct iterator_fixed_stride
        {
            using type = iterator_fixed_stride;
            using step_pointer_type = const std::ptrdiff_t*;

        private:
            std::ptrdiff_t m_step = 1;

        public:
            constexpr iterator_fixed_stride() noexcept { }

            /*implicit*/ constexpr iterator_fixed_stride(std::ptrdiff_t step) noexcept
                : m_step(step)
            {
            } // iterator_fixed_stride(...)

            constexpr bool contiguous() const noexcept
            {
                return this->m_step == 0 || this->m_step == 1 || this->m_step == -1;
            } // contiguous(...)

            step_pointer_type data() const noexcept { return &(this->m_step); }

            void increment(step_pointer_type& /*current*/) const noexcept { }
        }; // struct iterator_fixed_stride

        /** Variable step stride. */
        struct iterator_seq_stride
        {
            using type = iterator_seq_stride;
            using step_pointer_type = const std::ptrdiff_t*;

        private:
            std::vector<std::ptrdiff_t> m_steps = {};

        public:
            iterator_seq_stride(type&& other) noexcept
                : m_steps(std::move(other.m_steps))
            {
            } // iterator_seq_stride(...)

            type& operator =(type&& other) noexcept
            {
                this->m_steps = std::move(other.m_steps);
                return *this;
            } // operator =(...)

            /*implicit*/ iterator_seq_stride(std::vector<std::ptrdiff_t>&& steps) noexcept
                : m_steps(std::move(steps))
            {
            } // iterator_seq_stride(...)

            constexpr bool contiguous() const noexcept { return false; }

            step_pointer_type data() const noexcept { return this->m_steps.data(); }

            void increment(step_pointer_type& current) const noexcept
            {
                ++current;
            } // increment(...)
        }; // struct iterator_seq_stride
    } // namespace detail
} // namespace ropufu::aftermath::algebra

#endif // ROPUFU_AFTERMATH_ALGEBRA_ITERATOR_STRIDE_HPP_INCLUDED
