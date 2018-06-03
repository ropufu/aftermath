
#ifndef ROPUFU_AFTERMATH_FORMAT_MATHEADER_V4_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MATHEADER_V4_HPP_INCLUDED

#include "matstream.hpp"

#include <cstddef>   // std::size_t
#include <cstdint>   // std::int32_t
#include <fstream>   // std::ifstream, std::ofstream
#include <iostream>  // ??
#include <sstream>   // std::ostringstream
#include <stdexcept> // std::runtime_error
#include <string>    // std::string
#include <vector>    // std::vector

namespace ropufu::aftermath::format
{
    /** Indicates how the data are stored in a .mat file. */
    enum struct mat4_data_format : std::int32_t
    {
        ieee_little_endian = 0000,
        ieee_big_endian = 1000,
        vax_d_float = 2000,
        vax_g_float = 3000,
        cray = 4000
    }; // enum struct mat4_data_format

    /** Indicates what type of data is stored in a .mat file. */
    template <typename t_data_type>
    struct mat4_data_type_id;

    /** Indicates that a .mat file stores \c double. */
    template <> struct mat4_data_type_id<double> { static constexpr std::int32_t value = 00; };
    /** Indicates that a .mat file stores \c float. */
    template <> struct mat4_data_type_id<float> { static constexpr std::int32_t value = 10; };
    /** Indicates that a .mat file stores \c std::int32_t. */
    template <> struct mat4_data_type_id<std::int32_t> { static constexpr std::int32_t value = 20; };
    /** Indicates that a .mat file stores \c std::int16_t. */
    template <> struct mat4_data_type_id<std::int16_t> { static constexpr std::int32_t value = 30; };
    /** Indicates that a .mat file stores \c std::uint16_t. */
    template <> struct mat4_data_type_id<std::uint16_t> { static constexpr std::int32_t value = 40; };
    /** Indicates that a .mat file stores \c std::uint8_t. */
    template <> struct mat4_data_type_id<std::uint8_t> { static constexpr std::int32_t value = 50; };

    /** Indicates the type of matrix stored in a .mat file. */
    enum struct mat4_matrix_type_id : std::int32_t
    {
        full = 0,
        text = 1,
        sparse = 2
    }; // enum struct mat4_matrix_type_id

    /** @brief Header format for a .mat file. */
    template <>
    struct matheader<4>
    {
        using type = matheader<4>;
        static constexpr std::size_t mat_level = 4;

    private:
        std::int32_t m_data_format_id = 0;
        std::int32_t m_data_type_id = 0;
        std::int32_t m_matrix_type_id = 0;

        std::int32_t m_height = 0;
        std::int32_t m_width = 0;
        std::int32_t m_is_complex = 0;

        std::string m_name = "";

        /** Constructs a format type id from member fields. */
        std::int32_t build_format_type_id() const noexcept { return this->m_data_format_id + this->m_data_type_id + this->m_matrix_type_id; }

        /** Updates member fields from a format type id. */
        void decompose_format_type_id(std::int32_t format_type_id) noexcept
        {
            this->m_data_format_id = 1000 * (format_type_id / 1000);
            format_type_id -= this->m_data_format_id;

            this->m_data_type_id = 10 * (format_type_id / 10);
            format_type_id -= this->m_data_type_id;

            this->m_matrix_type_id = format_type_id;
        } // decompose_format_type_id(...)

    public:
        /** @brief Reads header from a .mat file.
         *  @return Number of bytes read.
         *  @exception std::runtime_error Specified file could not be opened.
         */
        std::size_t read(const std::string& file_path, std::size_t position)
        {
            std::size_t bytes_read = 0;
            std::ifstream filestream { };

            filestream.open(file_path.c_str(), std::ios::in | std::ios::binary);
            if (filestream.fail()) throw std::runtime_error("Failed to open file.");
            else
            {
                std::int32_t format_type_id = 0;
                std::int32_t height = 0;
                std::int32_t width = 0;
                std::int32_t complex_flag = 0;
                std::int32_t name_length = 0;
                char terminator = '\0';

                filestream.seekg(position);
                if (!filestream.good()) return 0;
                // Fixed-size header.
                filestream.read(reinterpret_cast<char*>(&format_type_id), sizeof(decltype(format_type_id))); // type
                if (!filestream.good()) return 0;
                filestream.read(reinterpret_cast<char*>(&height), sizeof(decltype(height))); // mrows
                if (!filestream.good() || height < 0) return 0;
                filestream.read(reinterpret_cast<char*>(&width), sizeof(decltype(width))); // ncols
                if (!filestream.good() || width < 0) return 0;
                filestream.read(reinterpret_cast<char*>(&complex_flag), sizeof(decltype(complex_flag))); // imagf
                if (!filestream.good()) return 0;
                filestream.read(reinterpret_cast<char*>(&name_length), sizeof(decltype(name_length))); // namlen
                if (!filestream.good() || name_length < 1) return 0;

                // Name.
                std::vector<char> text_data(name_length - 1);
                filestream.read(text_data.data(), name_length - 1);
                if (!filestream.good() || filestream.gcount() != name_length - 1) return 0;
                filestream.read(&terminator, 1);
                if (!filestream.good() || terminator != '\0') return 0;

                this->decompose_format_type_id(format_type_id);
                this->m_height = height;
                this->m_width = width;
                this->m_is_complex = (complex_flag == 0 ? false : true);
                this->m_name = std::string(text_data.begin(), text_data.end());
                bytes_read = this->size();
            } // if (...)

            return bytes_read;
        } // read(...)

        /** @brief Appends this header to a .mat file.
         *  @return Position in the file after writing the header.
         *  @exception std::runtime_error Specified file could not be opened.
         *  @todo Add write checks / verification.
         */
        std::size_t write(const std::string& filename) const
        {
            std::size_t position = 0;
            std::size_t existing_size = 0;
            std::ofstream filestream { };

            filestream.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary); // <std::ios::in> requires the file to exist.
            if (filestream.fail())
            {
                filestream.clear();
                filestream.open(filename.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc); // Create file if reading failed.
            } // if (...)
            
            if (filestream.fail()) throw std::runtime_error("Failed to open file.");
            else
            {
                std::int32_t format_type_id = this->build_format_type_id();
                std::int32_t height = static_cast<std::int32_t>(this->m_height);
                std::int32_t width = static_cast<std::int32_t>(this->m_width);
                std::int32_t complex_flag = this->m_is_complex ? 1 : 0;
                std::int32_t name_length = static_cast<std::int32_t>(this->m_name.size() + 1);
                char terminator = '\0';

                filestream.seekp(0, std::ios::end);
                existing_size = filestream.tellp();
                // Fixed-size header.
                filestream.write(reinterpret_cast<char*>(&format_type_id), sizeof(decltype(format_type_id))); // type
                filestream.write(reinterpret_cast<char*>(&height), sizeof(decltype(height))); // mrows
                filestream.write(reinterpret_cast<char*>(&width), sizeof(decltype(width))); // ncols
                filestream.write(reinterpret_cast<char*>(&complex_flag), sizeof(decltype(complex_flag))); // imagf
                filestream.write(reinterpret_cast<char*>(&name_length), sizeof(decltype(name_length))); // namlen

                // Name.
                filestream.write(this->m_name.c_str(), name_length - 1);
                filestream.write(&terminator, 1);

                position = existing_size + this->size();
            } // if (...)

            return position;
        } // write(...)

        /** Initializes the header for a given matrix. */
        template <typename t_matrix_type>
        void initialize(const t_matrix_type& mat,
            mat4_data_format data_format = mat4_data_format::ieee_little_endian, 
            mat4_matrix_type_id matrix_type_id = mat4_matrix_type_id::full) noexcept
        {
            using data_type = typename t_matrix_type::value_type;

            this->m_data_format_id = static_cast<std::int32_t>(data_format);
            this->m_data_type_id = mat4_data_type_id<data_type>::value;
            this->m_matrix_type_id = static_cast<std::int32_t>(matrix_type_id);

            this->m_height = static_cast<std::int32_t>(mat.height());
            this->m_width = static_cast<std::int32_t>(mat.width());
        } // initialize(...)

        /** Data format id. */
        std::int32_t data_format_id() const noexcept { return this->m_data_format_id; }

        /** Data type id. */
        std::int32_t data_type_id() const noexcept { return this->m_data_type_id; }

        /** Matrix type id. */
        std::int32_t matrix_type_id() const noexcept { return this->m_matrix_type_id; }

        /** Matrix height. */
        std::int32_t height() const noexcept { return this->m_height; }

        /** Matrix width. */
        std::int32_t width() const noexcept { return this->m_width; }

        /** Indicates if a matrix contains complex numbers. */
        bool is_complex() const noexcept { return this->m_is_complex; }

        /** Name of the matrix. */
        const std::string& name() const noexcept { return this->m_name; }

        /** Sets the name of the matrix to be written. */
        void set_name(const std::ostringstream& name_stream) noexcept { this->m_name = name_stream.str(); }
        /** Sets the name of the matrix to be written. */
        void set_name(const std::string& name) noexcept { this->m_name = name; }
        /** Sets the name of the matrix to be written. */
        void set_name(std::string&& name) noexcept { this->m_name = name; }

        /** Gets the size, in bytes, of the current header. */
        std::size_t size() const noexcept
        {
            return 5 * sizeof(std::int32_t) + this->m_name.size() + 1;
        } // size(...)
    }; // struct matheader<...>
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MATHEADER_V4_HPP_INCLUDED
