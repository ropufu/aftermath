
#ifndef ROPUFU_AFTERMATH_FORMAT_MAT4_OSTREAM_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MAT4_OSTREAM_HPP_INCLUDED

#include "../algebra/matrix.hpp"
#include "mat4_stream_base.hpp"

#include <cstddef>    // std::size_t
#include <cstdint>    // std::int32_t
#include <filesystem> // std::filesystem::path, std::filesystem::remove
#include <fstream>    // std::ofstream
#include <ios>        // std::ios_base::failure
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc

namespace ropufu::aftermath::format
{
    /** @brief Allows manipulation of the \c mat4_ostream objects. */
    enum struct mat4_ostream_manip : char
    {
        none, // Clears pending actions.
        wipe  // Indicates that file should be overwritten.
    }; // enum struct mat4_istream_manip

    /** @brief Handles writing matrices to MATLAB v4 .mat files. */
    struct mat4_ostream : public mat4_stream_base
    {
        using type = mat4_ostream;
        using header_type = typename mat4_stream_base::header_type;
        using arrangement_type = typename mat4_stream_base::arrangement_type;

        static constexpr std::int32_t mat_level = mat4_stream_base::mat_level;

    private:
        std::string m_next_variable_name = "";

        /** @brief Writes the matrix \p mat to \p filestream.
         *  @return Number of bytes written.
         */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        std::size_t write_to(std::ofstream& filestream,
            const aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat) noexcept
        {
            using data_type = t_value_type;

            std::size_t height = mat.height();
            std::size_t width = mat.width();
            std::size_t count = height * width;
            if (count == 0) return 0;

            arrangement_type arrangement { height, width };
            std::size_t first_position = filestream.tellp();
            std::size_t last_position = first_position + (count - 1) * sizeof(data_type);

            // Reserve file size.
            data_type current_value = 0;
            filestream.seekp(last_position);
            filestream.write(reinterpret_cast<const char*>(&current_value), sizeof(data_type));

            // Write the matrix.
            filestream.seekp(first_position);
            for (std::size_t blocks_processed = 0; blocks_processed < count; ++blocks_processed)
            {
                std::size_t column_index = 0;
                std::size_t row_index = 0;
                arrangement.reconstruct(blocks_processed, row_index, column_index);

                current_value = mat(row_index, column_index);
                filestream.write(reinterpret_cast<const char*>(&current_value), sizeof(data_type));
                if (filestream.fail())
                {
                    this->signal(std::errc::io_error);
                    return (blocks_processed * sizeof(data_type));
                } // if (...)
            } // for (...)
            return (count * sizeof(data_type));
        } // write_to(...)

    public:
        /** Creates an output stream for \p mat_path. */
        explicit mat4_ostream(const std::filesystem::path& mat_path) noexcept
            : mat4_stream_base(mat_path)
        {
        } // mat4_ostream(...)

        type& operator <<(mat4_ostream_manip flag) noexcept
        {
            if (this->fail()) return *this;
            
            switch (flag)
            {
                case mat4_ostream_manip::wipe:
                    this->wipe();
                    break;
                default: break;                
            } // switch (...)
            
            return *this;
        } // operator <<(...)

        type& operator <<(const std::string& variable_name) noexcept
        {
            if (this->fail()) return *this;
            
            this->m_next_variable_name = variable_name;
            return *this;
        } // operator <<(...)

        /** @brief Writes the matrix \p mat to the end of the .mat file. */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        type& operator <<(const aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat)
        {
            if (this->fail()) return *this;
            
            this->write(this->m_next_variable_name, mat);
            return *this;
        } // operator <<(...)

        /** @brief Writes the matrix \p mat to the end of the .mat file. */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        void write(const std::string& variable_name, const aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat)
        {
            if (this->fail()) return;

            // Initialize header.
            header_type header {};
            header.initialize(variable_name, mat);
            try
            {
                std::ofstream filestream {};

                // Try to open the existing file.
                filestream.open(this->path(), std::ios::in | std::ios::out | std::ios::binary); // std::ios::in requires the file to exist.
                if (filestream.fail())
                {
                    filestream.clear();
                    // Try to create a new file.
                    filestream.open(this->path(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc); // Create file if reading failed.
                } // if (...)
                if (filestream.fail())
                {
                    this->signal(std::errc::operation_not_permitted); // Failed to create file.
                    return;
                } // if (...)
                
                // Write header.
                filestream.seekp(0, std::ios::end);
                header.write(filestream, this->m_state);
                if (this->fail()) return; // Failed to write header.

                // Write body.
                this->write_to(filestream, mat);
                if (this->fail()) return; // Failed to write matrix.
            } // try
            catch (const std::ios_base::failure& /*e*/)
            {
                this->signal(std::errc::io_error);
                return;
            } // catch(...)

            // Reset name.
            this->m_next_variable_name.clear();
        } // write(...)

        /** @brief Erases the underlying .mat file. */
        void wipe() noexcept
        {
            if (this->fail()) return;
            std::filesystem::remove(this->path(), this->m_state);
        } // wipe(...)
    }; // struct mat4_ostream
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MAT4_OSTREAM_HPP_INCLUDED
