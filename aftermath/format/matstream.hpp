
#ifndef ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED

#include "../algebra/matrix.hpp"

#include <cstddef>
#include <string>

namespace ropufu::aftermath::format
{
    /** @brief Header format for \c matrstream. */
    template <std::size_t t_mat_level>
    struct matheader
    {
        static constexpr std::size_t mat_level = t_mat_level;
        using type = matheader<t_mat_level>;
    };

    /** @brief MATLAB file format for storing matrices.
     *  @remark This is a \c noexcept struct. Exception handling should be done by \c quiet_error singleton.
     */
    template <std::size_t t_mat_level>
    struct matstream
    {
        static constexpr std::size_t mat_level = t_mat_level;
        using type = matstream<t_mat_level>;
        using header_type = matheader<t_mat_level>;

        void clear() noexcept;

        type& operator <<(const std::string& name) noexcept;

        template <typename t_data_type>
        type& operator <<(const algebra::matrix<t_data_type>& matrix) noexcept;
    };
} // namespace ropufu::aftermath::format

namespace ropufu::afmt
{
    template <std::size_t t_mat_level>
    using matheader_t = ropufu::aftermath::format::matheader<t_mat_level>;

    template <std::size_t t_mat_level>
    using matstream_t = ropufu::aftermath::format::matstream<t_mat_level>;
} // namespace ropufu::afmt

#endif // ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
