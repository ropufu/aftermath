
#ifndef ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED

#include "../on_error.hpp"
#include "matheader_v4.hpp"
#include "matstream.hpp"

#include <cstddef>   // std::size_t
#include <cstdint>   // std::int32_t
#include <fstream>   // std::ifstream, std::ofstream
#include <ios>       // std::ios_base::failure
#include <iostream>  // ??
#include <sstream>   // std::ostringstream
#include <stdexcept> // std::runtime_error
#include <string>    // std::string
#include <system_error> // std::error_code, std::errc
#include <type_traits> // ...

namespace ropufu::aftermath::format
{
    /** @brief MATLAB file format for storing matrices.
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
    template <>
    struct matstream<4>
    {
        using type = matstream<4>;
        using header_type = matheader<4>;
        using arrangement_type = algebra::detail::column_major<std::size_t>;

        static constexpr std::int32_t mat_level = 4;

    private:
        std::string m_filename = "";
        std::size_t m_reader_position = 0;
        std::ostringstream m_name_stream = std::ostringstream();

    public:
        /** Creates a \c matstream for a given \p filename. */
        explicit matstream(const std::string& filename) noexcept
            : m_filename(filename)
        {
        } // matstream(...)

        /** @brief Clears the .mat file, and resets reader position.
         *  @param ec Set to \c std::errc::io_error if the underlying file could not be created.
         */
        void clear(std::error_code& ec) noexcept
        {
            try
            {
                std::ofstream filestream;
                filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
                if (filestream.fail()) return aftermath::detail::on_error(ec, std::errc::io_error, "Failed to create file.");
                this->m_reader_position = 0;
            } // try 
            catch (const std::ios_base::failure& e)
            {
                return aftermath::detail::on_error(ec, std::errc::io_error, e.what());
            } // catch(...)
        } // clear(...)

        /** @brief If \p value is a string or a number, appends the provided value to the name of the next matrix.
         *         Otherwise treats it as a matrix, and writes it to the end of the .mat file.
         *  @remark Advances current reader position to the end of the written block (end of the file).
         *  @exception std::runtime_error Underlying file could not be opened.
         */
        template <typename t_value_type>
        type& operator <<(const t_value_type& value)
        {
            constexpr bool is_arithmetic = std::is_arithmetic_v<t_value_type>;
            constexpr bool is_string_convertible = std::is_convertible_v<t_value_type, std::string>;
            if constexpr (is_arithmetic || is_string_convertible) this->m_name_stream << value;
            else
            {
                try
                {
                    std::ofstream filestream {};

                    // Initialize header.
                    header_type header {};
                    header.initialize(value);
                    header.set_name(this->m_name_stream);
                    // Reset name stream.
                    this->m_name_stream.clear();
                    this->m_name_stream.str("");
                    
                    // Write header.
                    std::error_code ec {};
                    std::size_t position = header.write(this->m_filename, ec);
                    if (position == 0 || ec) throw std::runtime_error("Failed to write header.");

                    // Write body.
                    ec.clear();
                    this->m_reader_position = this->write(value, position, ec);
                    if (ec) throw std::runtime_error("Failed to write matrix.");
                } // try 
                catch (const std::ios_base::failure& e)
                {
                    throw std::runtime_error(e.what());
                } // catch(...)
            } // if constexpr (...)
            return *this;
        } // operator <<(...)

        /** @brief Loads a matrix from a file.
         *  @remark Advances current reader position to the end of the read block.
         *  @param ec Set to std::errc::io_error if the header could not be read.
         *  @param ec Set to std::errc::bad_file_descriptor if data type in \p mat does not match that in the file.
         */
        template <typename t_matrix_type>
        void load(std::string& matrix_name, t_matrix_type& mat, std::error_code& ec) noexcept
        {
            using data_type = typename t_matrix_type::value_type;

            // Read header.
            header_type header {};
            std::size_t header_size = header.read(this->m_filename, this->m_reader_position, ec);
            if (header_size == 0 || ec) return aftermath::detail::on_error(ec, std::errc::io_error, "Failed to read header.");
            matrix_name = header.name();

            if (mat4_data_type_id<data_type>::value != header.data_type_id()) return aftermath::detail::on_error(ec, std::errc::bad_file_descriptor, "Matrix data type mismatch.");

            std::size_t height = header.height();
            std::size_t width = header.width();
            mat = t_matrix_type(height, width);
            
            // Read body.
            ec.clear();
            this->m_reader_position = this->read(mat, this->m_reader_position + header_size, ec);
        } // load(...)

    private:
        /** @brief Writes \p matrix to the .mat file at position \p position.
         *  @return Position (in bytes) at the end of the written block (end of the file).
         *  @param ec Set to std::errc::io_error if the underlying file could not be opened.
         */
        template <typename t_matrix_type>
        std::size_t write(const t_matrix_type& mat, std::size_t position, std::error_code& ec) const noexcept
        {
            using data_type = typename t_matrix_type::value_type;
            try
            {
                std::ofstream filestream {};
                
                // Write body.
                filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::out | std::ios::binary); // File must already exist: the header has been written.
                if (filestream.fail()) return aftermath::detail::on_error(ec, std::errc::io_error, "Failed to open file.", position);
                else
                {
                    std::size_t height = mat.height();
                    std::size_t width = mat.width();

                    data_type current_value = 0;
                    for (std::size_t row_index = 0; row_index < height; ++row_index)
                    {
                        for (std::size_t column_index = 0; column_index < width; ++column_index)
                        {
                            current_value = mat(row_index, column_index);
                            filestream.seekp(position + arrangement_type::flatten(row_index, column_index, height, width) * sizeof(data_type));
                            filestream.write(reinterpret_cast<const char*>(&current_value), sizeof(data_type));
                        } // for (...)
                    } // for (...)
                    return position + (height * width * sizeof(data_type));
                } // if (...)
            } // try 
            catch (const std::ios_base::failure& e)
            {
                return aftermath::detail::on_error(ec, std::errc::io_error, e.what(), position);
            } // catch(...)
        } // write(...)

        /** @brief Reads the .mat file at position \p position, stores the result in \p matrix.
         *  @return Position (in bytes) at the end of the read block.
         *  @param ec Set to std::errc::io_error if the underlying file could not be opened.
         */
        template <typename t_matrix_type>
        std::size_t read(t_matrix_type& mat, std::size_t position, std::error_code& ec) noexcept
        {
            using data_type = typename t_matrix_type::value_type;
            try
            {
                std::ifstream filestream {};

                // Read body.
                filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::binary);
                if (filestream.fail()) return aftermath::detail::on_error(ec, std::errc::io_error, "Failed to open file.", position);
                else
                {
                    std::size_t height = mat.height();
                    std::size_t width = mat.width();

                    data_type current_value = 0;
                    for (std::size_t row_index = 0; row_index < height; ++row_index)
                    {
                        for (std::size_t column_index = 0; column_index < width; ++column_index)
                        {
                            filestream.seekg(position + arrangement_type::flatten(row_index,  column_index, height, width) * sizeof(data_type));
                            filestream.read(reinterpret_cast<char*>(&current_value), sizeof(data_type));
                            mat(row_index, column_index) = current_value;
                        } // for (...)
                    } // for (...)
                    return position + (height * width * sizeof(data_type));
                } // if (...)
            } // try 
            catch (const std::ios_base::failure& e)
            {
                return aftermath::detail::on_error(ec, std::errc::io_error, e.what(), position);
            } // catch(...)
        } // read(...)
    }; // struct matstream<...>
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED
