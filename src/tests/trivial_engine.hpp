
#ifndef ROPUFU_AFTERMATH_TESTS_TRIVIAL_ENGINE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_TRIVIAL_ENGINE_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <limits>  // std::numeric_limits
#include <random>  // std::seed_seq

namespace ropufu::tests
{
    template <typename t_result_type,
        t_result_type t_min = std::numeric_limits<t_result_type>::min(),
        t_result_type t_max = std::numeric_limits<t_result_type>::max(),
        t_result_type t_state = t_min>
    struct trivial_engine
    {
        using type = trivial_engine<t_result_type, t_min, t_max, t_state>;
        using result_type = t_result_type;

        explicit constexpr trivial_engine() noexcept { }
        explicit constexpr trivial_engine(result_type /*seed*/) noexcept { }
        explicit constexpr trivial_engine(const std::seed_seq& /*seq*/) noexcept { }

        static constexpr result_type min() noexcept { return t_min; }
        static constexpr result_type max() noexcept { return t_max; }

        constexpr void seed(result_type /*const_value*/) noexcept { }
        constexpr void seed(const std::seed_seq& /*seq*/) noexcept { }

        constexpr void discard(std::size_t /*skip*/) const noexcept { }

        constexpr result_type operator ()() const noexcept { return t_state; }
    }; // enum struct trivial_engine
} // namespace ropufu::tests

#endif // ROPUFU_AFTERMATH_TESTS_TRIVIAL_ENGINE_HPP_INCLUDED