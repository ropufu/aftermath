
#ifndef ROPUFU_AFTERMATH_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED

#include "../algebra/matrix_arrangement.hpp"
#include "mat4_header.hpp"

#include <cstddef>      // std::size_t
#include <cstdint>      // std::int32_t
#include <filesystem>   // std::filesystem::path
#include <system_error> // std::error_code, std::errc, std::make_error_code

namespace ropufu::aftermath::format
{
    /** @brief Handles streaming matrices to and from MATLAB v4 .mat files.
     *
     *  A MAT-file may contain one or more matrices. The matrices are written
     *  sequentially on disk, with the bytes forming a continuous stream. Each
     *  matrix starts with a fixed-length 20-byte header that contains information
     *  describing certain attributes of the Matrix. The 20-byte header consists of five
     *  4-byte integers:
     *  1) type (mat4_data_format + mat4_data_type_id::value + mat4_matrix_type_id);
     *  2) mrows (number of rows in the matrix);
     *  3) ncols (number of columns in the matrix);
     *  4) imagf (1 if the matrix has an imaginary part, 0 otherwise);
     *  5) namlen (length of the matrix name plus 1).
     *
     *  Immediately following the fixed length header is the data whose length is
     *  dependent on the variables in the fixed length header:
     *  1) name : <namelen> bytes.
     *     ASCII bytes, the last one of which must be a null character '\0';
     *  2) real : <mrows> * <ncols> numbers, size depends on <mat4_data_type>.
     *     Real part of the matrix, with data stored column-wise.
     *     For example, a matrix
     *     || a b c ||
     *     || d e f ||
     *     would be written as "adbecf".
     *  3) imag : <imagf> * <mrows> * <ncols> numbers, size depends on <mat4_data_type>.
     *     Imaginary part of the matrix, with data stored column-wise.
     */
    struct mat4_stream_base
    {
        using type = mat4_stream_base;
        using header_type = mat4_header;
        using arrangement_type = aftermath::algebra::detail::column_major<std::size_t>;
        
        static constexpr std::int32_t mat_level = 4;

    private:
        std::filesystem::path m_mat_path = {};

    protected:
        std::error_code m_state = {};

        void signal(std::errc code)
        {
            this->m_state = std::make_error_code(code);
        } // signal(...)

    public:
        /** Creates a \c matstream for a given \p filename. */
        explicit mat4_stream_base(const std::filesystem::path& mat_path) noexcept
            : m_mat_path(mat_path)
        {
        } // mat4_ostream(...)

        const std::filesystem::path& path() const noexcept { return this->m_mat_path; }

        const std::error_code& state() const noexcept { return this->m_state; }

        bool good() const noexcept { return this->m_state.value() == 0; }

        bool fail() const noexcept { return !this->good(); }

        /** Clears the state of the stream. */
        void clear() noexcept
        {
            this->m_state.clear();
        } // clear(...)
    }; // struct mat4_stream_base
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MAT4_STREAM_BASE_HPP_INCLUDED
