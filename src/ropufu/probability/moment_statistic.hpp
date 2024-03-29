
#ifndef ROPUFU_AFTERMATH_PROBABILITY_MOMENT_STATISTIC_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_MOMENT_STATISTIC_HPP_INCLUDED

#include "../concepts.hpp"
#include "../number_traits.hpp"

#include <array>   // std::array
#include <cstddef> // std::size_t
#include <type_traits> // std::void_t

namespace ropufu::aftermath::probability
{
    namespace detail
    {
        template <typename t_type, typename = void>
        struct vector_to_scalar
        {
            using scalar_type = t_type;
        }; // struct vector_to_scalar

        template <typename t_type>
        struct vector_to_scalar<t_type, std::void_t<typename t_type::value_type>>
        {
            using scalar_type = typename t_type::value_type;
        }; // struct vector_to_scalar<...>

        template <typename t_type>
        using vector_to_scalar_t = typename vector_to_scalar<t_type>::scalar_type;
    } // namespace detail
    
    /** @brief A fast statistic builder to keep track of means and variances.
     *  @todo Constraint types based on required arithmetic/scalar operations.
     */
    template <ropufu::ring t_observation_type, typename t_statistic_type = t_observation_type, std::size_t t_order = 3>
    struct moment_statistic
    {
        using type = moment_statistic<t_observation_type, t_statistic_type, t_order>;
        using observation_type = t_observation_type;
        using statistic_type = t_statistic_type;
        using scalar_type = detail::vector_to_scalar_t<statistic_type>;

        /** Number of bins. */
        static constexpr std::size_t breadth = t_order + 1;

        template <typename t_type>
        using bins_t = std::array<t_type, type::breadth>;

    private:
        std::size_t m_count = 0; // Total count of observations.
        statistic_type m_zero = {}; // Auxiliary "zero" structure: necessary to maintain consistent matrix sizes.
        statistic_type m_shift = {}; // Shift to offset every observation.
        std::size_t m_bin_index = 0; // Pointer to the next bin to be filled.
        bins_t<statistic_type> m_local_shifted_sums = {}; // sum(x - shift) = n (mean - shift).
        bins_t<statistic_type> m_local_shifted_squares = {}; // sum(x - shift)^2 = (n - 1) var + sum(x - shift) / n.

        /** Indicates if all bins have the same counts. */
        bool is_balanced() const noexcept { return this->m_bin_index == 0; }

    public:
        moment_statistic() noexcept { }

        explicit moment_statistic(const statistic_type& anticipated_mean) noexcept
            : m_zero(anticipated_mean), m_shift(anticipated_mean)
        {
            this->m_zero = 0;
            this->m_local_shifted_sums.fill(this->m_zero);
            this->m_local_shifted_squares.fill(this->m_zero);
        } // moment_statistic(...)

        const statistic_type& shift() const noexcept { return this->m_shift; }

        void clear() noexcept
        {
            this->m_count = 0;

            this->m_bin_index = 0;
            this->m_local_shifted_sums.fill(this->m_zero);
            this->m_local_shifted_squares.fill(this->m_zero);
        } // clear(...)

        void observe(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::breadth; ++i)
            {
                this->m_local_shifted_sums[i] += other.m_local_shifted_sums[i];
                this->m_local_shifted_squares[i] += other.m_local_shifted_squares[i];
            } // for (...)
            this->m_count += other.m_count;
        } // observe(...)

        void observe(const observation_type& value) noexcept
        {
            statistic_type x = static_cast<statistic_type>(value);
            x -= this->m_shift; // x now holds the offset value.

            this->m_local_shifted_sums[this->m_bin_index] += x;

            x *= x; // x now holds the offset value squared.
            this->m_local_shifted_squares[this->m_bin_index] += x;

            ++this->m_count;
            this->m_bin_index = (this->m_bin_index + 1) % (type::breadth);
        } // observe(...)
            
        type& operator <<(const observation_type& value) noexcept
        {
            this->observe(value);
            return *this;
        } // operator <<(...)

        std::size_t count() const noexcept { return this->m_count; }

        bool empty() const noexcept { return this->m_count == 0; }

        statistic_type mean() const noexcept
        {
            scalar_type n = static_cast<scalar_type>(this->m_count);

            // S = sum(x - shift) = n (mean - shift).
            // mean = shift + (S / n) = shift + sum(S_local / n).
            statistic_type mean { this->m_shift };
            for (std::size_t j = 0; j < type::breadth; ++j)
            {
                statistic_type s { this->m_local_shifted_sums[j] };
                s /= n;
                mean += s;
            } // for (...)
            return mean;
        } // mean(...)

        statistic_type variance() const noexcept
        {
            if (this->m_count == 0) return this->m_zero;

            scalar_type n = static_cast<scalar_type>(this->m_count);
            scalar_type n_less_one = static_cast<scalar_type>(this->m_count - 1);

            // Q = sum(x - shift)^2.
            // S = sum(x - shift).
            // (n - 1) var = Q - n (mean - shift)^2.
            // (n - 1) var = Q - S^2 / n.
            // var = Q / (n - 1) - [S / (n - 1)] [S / n]
            //     = sum(Q_local / (n - 1)) - [sum(S_local / (n - 1))] [sum(S_local / n)].
            statistic_type variance { this->m_zero };
            statistic_type variance_sa { this->m_zero };
            statistic_type variance_sb { this->m_zero };

            for (std::size_t j = 0; j < type::breadth; ++j)
            {
                statistic_type q { this->m_local_shifted_squares[j] };
                statistic_type sa { this->m_local_shifted_sums[j] };
                statistic_type sb { this->m_local_shifted_sums[j] };

                q /= n_less_one;
                sa /= n_less_one;
                sb /= n;

                variance += q;
                variance_sa += sa;
                variance_sb += sb;
            } // for (...)

            variance_sa *= variance_sb;
            variance -= variance_sa;
            aftermath::make_non_negative(variance);
            return variance;
        } // variance(...)
    }; // struct moment_statistic
} // namespace ropufu::aftermath::probability

#endif // ROPUFU_AFTERMATH_PROBABILITY_MOMENT_STATISTIC_HPP_INCLUDED
