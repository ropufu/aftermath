
#ifndef ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED

#include "../algebra/matrix.hpp"

#include <cstddef>
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        namespace format
        {
            /** @brief Header format for \c matrstream. */
            template <std::size_t t_mat_level>
            struct matheader
            {
                static constexpr std::size_t mat_level = t_mat_level;
            };

            /** @brief MATLAB file format for storing matrices. */
            template <std::size_t t_mat_level>
            struct matstream
            {
                static constexpr std::size_t mat_level = t_mat_level;
                typedef matstream<t_mat_level> type;
                typedef matheader<t_mat_level> header_type;

                void clear();

                type& operator <<(const std::string& name);

                template <typename t_data_type>
                type& operator <<(const algebra::matrix<t_data_type>& matrix);
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_FORMAT_MATSTREAM_HPP_INCLUDED
