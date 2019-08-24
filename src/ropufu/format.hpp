
#ifndef ROPUFU_AFTERMATH_FORMAT_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_HPP_INCLUDED

#include "format/cat.hpp"
#include "format/mat4_header.hpp"
#include "format/mat4_istream.hpp"
#include "format/mat4_ostream.hpp"

#include <string>  // std::string
#include <utility> // std::forward

namespace ropufu::aftm
{
    template <typename... t_arg_types>
    static std::string cat(t_arg_types&&... args)
    {
        return aftermath::format::cat(std::forward<t_arg_types>(args)...);
    } // cat(...)

    using mat4_header_t = aftermath::format::mat4_header;
    using mat4_istream_t = aftermath::format::mat4_istream;
    using mat4_ostream_t = aftermath::format::mat4_ostream;
} // namespace ropufu::aftm

#endif // ROPUFU_AFTERMATH_FORMAT_HPP_INCLUDED
