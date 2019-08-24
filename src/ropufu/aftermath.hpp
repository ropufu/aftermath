
#ifndef ROPUFU_AFTERMATH_HPP_INCLUDED
#define ROPUFU_AFTERMATH_HPP_INCLUDED

#include "enum_array.hpp"
#include "enum_parser.hpp"
#include "key_value_pair.hpp"
#include "math_constants.hpp"

#include "algebra.hpp"
#include "algorithm.hpp"
#include "format.hpp"
#include "probability.hpp"
#include "random.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    /** @brief An array indexed by \tparam t_enum_type. */
    template <typename t_enum_type, typename t_value_type>
    using enum_array_t = aftermath::enum_array<t_enum_type, t_value_type>;

    /** @brief Masks enumerable keys of \tparam t_enum_type. */
    template <typename t_enum_type>
    using flags_t = aftermath::flags_t<t_enum_type>;

    template <typename t_numeric_type>
    using math_constants_t = aftermath::math_constants<t_numeric_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_HPP_INCLUDED
