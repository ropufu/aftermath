
#ifndef ROPUFU_AFTERMATH_HPP_INCLUDED
#define ROPUFU_AFTERMATH_HPP_INCLUDED

#include "algebra.hpp"
#include "algorithm.hpp"
#include "format.hpp"
#include "probability.hpp"
#include "random.hpp"

#include "enum_array.hpp"
#include "enum_parser.hpp"
#include "key_value_pair.hpp"
#include "math_constants.hpp"
#include "type_traits.hpp"

/** Shorthand notation. */
namespace ropufu::aftm
{
    /** @brief An array indexed by \tparam t_enum_type. */
    template <typename t_enum_type, typename t_value_type>
    using enum_array_t = ropufu::aftermath::enum_array<t_enum_type, t_value_type>;

    /** @brief Masks enumerable keys of \tparam t_enum_type. */
    template <typename t_enum_type>
    using flags_t = ropufu::aftermath::flags_t<t_enum_type>;

    template <typename t_numeric_type>
    using math_constants_t = ropufu::aftermath::math_constants<t_numeric_type>;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_HPP_INCLUDED
