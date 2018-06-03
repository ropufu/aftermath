
#ifndef ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <string>  // std::string

namespace ropufu::aftermath::format
{
    /** @brief Header format for \c matrstream. */
    template <std::size_t t_mat_level>
    struct matheader
    {
        using type = matheader<t_mat_level>;
        static constexpr std::size_t mat_level = t_mat_level;
    }; // struct matheader

    /** @brief MATLAB file format for storing matrices. */
    template <std::size_t t_mat_level>
    struct matstream
    {
        using type = matstream<t_mat_level>;
        using header_type = matheader<t_mat_level>;
        static constexpr std::size_t mat_level = t_mat_level;

        void clear();

        template <typename t_value_type>
        type& operator <<(const t_value_type& value);
    };
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
