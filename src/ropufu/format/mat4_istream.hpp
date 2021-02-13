
#ifndef ROPUFU_AFTERMATH_FORMAT_MAT4_ISTREAM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MAT4_ISTREAM_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "mat4_header.hpp"
#include "mat4_stream_base.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int32_t
#include <filesystem> // std::filesystem::path
#include <fstream>    // std::ifstream
#include <ios>        // std::ios_base::failure
#include <optional>   // std::optional, std::nullopt
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc

namespace ropufu::aftermath::format
{
    /** @brief Allows manipulation of the \c mat4_istream objects. */
    enum struct mat4_istream_manip : char
    {
        none, // Clears pending actions.
        skip  // Indicates that the next variable should be skipped.
    }; // enum struct mat4_istream_manip

    /** @brief Handles reading matrices to MATLAB v4 .mat files. */
    struct mat4_istream : public mat4_stream_base
    {
        using type = mat4_istream;
        using header_type = typename mat4_stream_base::header_type;
        using arrangement_type = typename mat4_stream_base::arrangement_type;

        static constexpr std::int32_t mat_level = mat4_stream_base::mat_level;

    private:
        std::size_t m_next_block_position = 0;

        /** @brief Reads the matrix \p mat from \p filestream.
         *  @return Number of bytes read.
         */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        std::size_t read_from(std::ifstream& filestream,
            aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat) noexcept
        {
            using data_type = t_value_type;

            std::size_t height = mat.height();
            std::size_t width = mat.width();
            std::size_t count = height * width;
            if (count == 0) return 0;

            arrangement_type arrangement { height, width };
            data_type current_value = 0;
            for (std::size_t blocks_processed = 0; blocks_processed < count; ++blocks_processed)
            {
                std::size_t column_index = 0;
                std::size_t row_index = 0;
                arrangement.reconstruct(blocks_processed, row_index, column_index);
                
                filestream.read(reinterpret_cast<char*>(&current_value), sizeof(data_type));
                if (filestream.fail())
                {
                    this->signal(std::errc::io_error);
                    return (blocks_processed * sizeof(data_type));
                } // if (...)
                mat(row_index, column_index) = current_value;
            } // for (...)
            return (count * sizeof(data_type));
        } // read_from(...)

        /** @brief Reads the header at the current position in file and optionally jumps to the next header. */
        void peek(header_type& header, bool skip_to_next)
        {
            try
            {
                std::ifstream filestream {};

                // Read body.
                filestream.open(this->path(), std::ios::in | std::ios::binary);
                if (filestream.fail())
                {
                    this->signal(std::errc::operation_not_permitted); // Failed to open file.
                    return;
                } // if (...)
                
                // Read header.
                filestream.seekg(this->m_next_block_position);
                header.read(filestream, this->m_state);
                if (this->fail()) return; // Failed to read header.
                
                if (skip_to_next)
                {
                    std::size_t data_type_size = mat4_data_type_size_by_id(header.data_type_id());
                    if (data_type_size == 0)
                    {
                        this->signal(std::errc::bad_file_descriptor); // Data type not recognized.
                        return;
                    } // if (...)
                    std::size_t height = header.height();
                    std::size_t width = header.width();
                    std::size_t count = height * width;

                    this->m_next_block_position += header.size() + (count * data_type_size);
                } // if (...)
            } // try
            catch (const std::ios_base::failure& /*e*/)
            {
                this->signal(std::errc::io_error);
                return;
            } // catch(...)
        } // peek(...)

    public:
        /** Creates an input stream for \p mat_path. */
        explicit mat4_istream(const std::filesystem::path& mat_path) noexcept
            : mat4_stream_base(mat_path)
        {
        } // mat4_istream(...)

        type& operator >>(mat4_istream_manip flag) noexcept
        {
            if (this->fail()) return *this;
            
            header_type header {};
            switch (flag)
            {
                case mat4_istream_manip::skip:
                    this->peek(header, true);
                    break;
                default: break;
            } // switch (...)

            return *this;
        } // operator >>(...)

        type& operator >>(std::string& variable_name) noexcept
        {
            if (this->fail()) return *this;
            
            header_type header {};
            this->peek(header, false);
            if (this->fail()) return *this;

            variable_name = header.name();
            return *this;
        } // operator >>(...)

        /** @brief Writes the matrix \p mat to the end of the .mat file. */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        type& operator >>(aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat)
        {
            if (this->fail()) return *this;
            
            std::string variable_name {};
            this->read(variable_name, mat);
            return *this;
        } // operator >>(...)

        /** @brief Reads a matrix from the current position file and stores it in \p mat. */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        void read(std::string& variable_name, aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat) noexcept
        {
            using data_type = t_value_type;
            using matrix_type = aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>;

            if (this->fail()) return;

            try
            {
                std::ifstream filestream {};

                // Read body.
                filestream.open(this->path(), std::ios::in | std::ios::binary);
                if (filestream.fail())
                {
                    this->signal(std::errc::operation_not_permitted); // Failed to open file.
                    return;
                } // if (...)
                
                // Read header.
                filestream.seekg(this->m_next_block_position);
                header_type header {};
                header.read(filestream, this->m_state);
                if (this->fail()) return; // Failed to read header.

                variable_name = header.name();
                if (mat4_data_type_id<data_type>::value != header.data_type_id())
                {
                    this->signal(std::errc::bad_file_descriptor); // Matrix data type mismatch.
                    return;
                } // if (...)
                std::size_t height = header.height();
                std::size_t width = header.width();
                mat = matrix_type::uninitialized(height, width);
                    
                // Read body.
                this->read_from(filestream, mat);
                if (filestream.fail()) this->signal(std::errc::io_error);
                if (this->fail()) return; // Failed to read matrix.
                
                this->m_next_block_position = filestream.tellg();
            } // try
            catch (const std::ios_base::failure& /*e*/)
            {
                this->signal(std::errc::io_error);
                return;
            } // catch(...)
        } // read(...)
    }; // struct mat4_istream
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MAT4_ISTREAM_HPP_INCLUDED
