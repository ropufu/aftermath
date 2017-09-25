
#ifndef ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "matheader_v4.hpp"
#include "matstream.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace format
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
                typedef typename algebra::detail::matrix_arrangement<false> arrangement_type;
                typedef matstream<4> type;
                typedef matheader<4> header_type;
                static constexpr std::int32_t mat_level = 4;

            private:
                std::string m_filename;
                std::size_t m_reader_position;
                std::ostringstream m_name_stream;

            public:
                /** Creates a \c matstream for a given \p filename. */
                explicit matstream(const std::string& filename) noexcept
                    : m_filename(filename), m_reader_position(0), m_name_stream()
                {
                }

                /** @brief Clears the .mat file, and resets reader position.
                 *  @exception std::runtime_error Underlying file could not be created.
                 */
                void clear()
                {
                    std::ofstream filestream;
                    filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
                    if (filestream.fail()) throw std::runtime_error("Failed to create file.");
                    else filestream.close();
                    this->m_reader_position = 0;
                }

                /** Appends the provided value to the name of the next matrix. */
                type& operator <<(const std::string& name)
                {
                    this->m_name_stream << name;
                    return *this;
                }

                /** Appends the provided value to the name of the next matrix. */
                type& operator <<(std::string&& name)
                {
                    this->m_name_stream << name;
                    return *this;
                }

                /** Appends the provided value to the name of the next matrix. */
                template <typename t_name_type>
                std::enable_if_t<std::is_integral<t_name_type>::value, type>& operator <<(const t_name_type& name)
                {
                    this->m_name_stream << name;
                    return *this;
                }

                /** Appends the provided value to the name of the next matrix. */
                template <typename t_name_type>
                std::enable_if_t<std::is_integral<t_name_type>::value, type>& operator <<(t_name_type&& name)
                {
                    this->m_name_stream << name;
                    return *this;
                }

                /** @brief Writes \p matrix to the end of .mat file.
                 *  @remark Advances current reader position to the end of the written block (end of the file).
                 *  @exception std::runtime_error Underlying file could not be opened.
                 */
                template <typename t_data_type, bool t_is_row_major>
                type& operator <<(const algebra::matrix<t_data_type, t_is_row_major>& mat)
                {
                    std::ofstream filestream;

                    // Initialize header.
                    header_type header;
                    header.initialize(mat);
                    header.set_name(this->m_name_stream);
                    // Reset name stream.
                    this->m_name_stream.clear();
                    this->m_name_stream.str("");
                    
                    // Write header.
                    std::size_t position = header.write(this->m_filename);
                    // Write body.
                    this->m_reader_position = this->write(mat, position);

                    return *this;
                }

                /** @brief Loads a matrix from a file.
                 *  @remark Advances current reader position to the end of the read block.
                 *  @exception std::runtime_error Failed to read header.
                 *  @exception std::runtime_error Data type mismatch in file and provided \p filename.
                 */
                template <typename t_data_type, bool t_is_row_major>
                void load(std::string& matrix_name, algebra::matrix<t_data_type, t_is_row_major>& mat)
                {
                    typedef t_data_type data_type;

                    // Read header.
                    header_type header;
                    std::size_t header_size = header.read(this->m_filename, this->m_reader_position);
                    if (header_size == 0) throw std::runtime_error("Failed to read header.");
                    matrix_name = header.name();

                    if (mat4_data_type_id<data_type>::value != header.data_type_id()) throw std::runtime_error("Matrix data type mismatch.");

                    std::size_t height = header.height();
                    std::size_t width = header.width();
                    mat.resize(height, width);
                    
                    // Read body.
                    this->m_reader_position = this->read(mat, this->m_reader_position + header_size);
                }

            private:
                /** @brief Writes \p matrix to the .mat file at position \p position.
                 *  @return Position (in bytes) at the end of the written block (end of the file).
                 *  @exception std::runtime_error Underlying file could not be opened.
                 */
                template <typename t_data_type, bool t_is_row_major>
                std::size_t write(const algebra::matrix<t_data_type, t_is_row_major>& mat, std::size_t position)
                {
                    typedef t_data_type data_type;
                    std::ofstream filestream;
                    
                    // Write body.
                    filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::out | std::ios::binary);
                    if (filestream.fail()) throw std::runtime_error("Failed to open file.");
                    else
                    {
                        std::size_t height = mat.height();
                        std::size_t width = mat.width();

                        data_type current_value;
                        for (std::size_t row_index = 0; row_index < height; row_index++)
                        {
                            for (std::size_t column_index = 0; column_index < width; column_index++)
                            {
                                current_value = mat(row_index, column_index);
                                filestream.seekp(position + arrangement_type::flatten(row_index, column_index, height, width) * sizeof(data_type));
                                filestream.write(reinterpret_cast<const char*>(&current_value), sizeof(data_type));
                            }
                        }
                        filestream.close();
                        return position + (height * width * sizeof(data_type));
                    }
                    //return position;
                }

                /** @brief Reads the .mat file at position \p position, stores the result in \p matrix.
                 *  @return Position (in bytes) at the end of the read block.
                 *  @exception std::runtime_error Underlying file could not be opened.
                 */
                template <typename t_data_type, bool t_is_row_major>
                std::size_t read(algebra::matrix<t_data_type, t_is_row_major>& mat, std::size_t position)
                {
                    typedef t_data_type data_type;
                    std::ifstream filestream;

                    // Read body.
                    filestream.open(this->m_filename.c_str(), std::ios::in | std::ios::binary);
                    if (filestream.fail()) throw std::runtime_error("Failed to open file.");
                    else
                    {
                        std::size_t height = mat.height();
                        std::size_t width = mat.width();

                        data_type current_value;
                        for (std::size_t row_index = 0; row_index < height; row_index++)
                        {
                            for (std::size_t column_index = 0; column_index < width; column_index++)
                            {
                                filestream.seekg(position + arrangement_type::flatten(row_index,  column_index, height, width) * sizeof(data_type));
                                filestream.read(reinterpret_cast<char*>(&current_value), sizeof(data_type));
                                mat(row_index, column_index) = current_value;
                            }
                        }
                        filestream.close();
                        return position + (height * width * sizeof(data_type));
                    }
                    //return position;
                }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_FORMAT_MATSTREAM_V4_HPP_INCLUDED
