
#ifndef ROPUFU_AFTERMATH_FORMAT_MAT4_HEADER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_FORMAT_MAT4_HEADER_HPP_INCLUDED

#include "../algebra/matrix.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <fstream> // std::ifstream, std::ofstream
#include <ios>     // std::ios_base::failure
#include <string>  // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>  // std::vector

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

    [[maybe_unused]]
    static std::size_t mat4_data_type_size_by_id(std::int32_t data_type_id) noexcept
    {
        switch (data_type_id)
        {
            case mat4_data_type_id<double>::value: return sizeof(double); break;
            case mat4_data_type_id<float>::value: return sizeof(float); break;
            case mat4_data_type_id<std::int32_t>::value: return sizeof(std::int32_t); break;
            case mat4_data_type_id<std::int16_t>::value: return sizeof(std::int16_t); break;
            case mat4_data_type_id<std::uint16_t>::value: return sizeof(std::uint16_t); break;
            case mat4_data_type_id<std::uint8_t>::value: return sizeof(std::uint8_t); break;
        } // switch (...)
        return 0;
    } // mat4_data_type_size_by_id(...)

    /** Indicates the type of matrix stored in a .mat file. */
    enum struct mat4_matrix_type_id : std::int32_t
    {
        full = 0,
        text = 1,
        sparse = 2
    }; // enum struct mat4_matrix_type_id

    /** @brief Header format for a .mat file. */
    struct mat4_header
    {
        using type = mat4_header;
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

        std::size_t on_read_error(std::error_code& ec, std::size_t int32_blocks_processed) const noexcept
        {
            ec = std::make_error_code(std::errc::io_error);
            return int32_blocks_processed * sizeof(std::int32_t);
        } // on_read_error(...)

        std::size_t on_write_error(std::error_code& ec, std::size_t int32_blocks_processed) const noexcept
        {
            ec = std::make_error_code(std::errc::io_error);
            return int32_blocks_processed * sizeof(std::int32_t);
        } // on_write_error(...)

    public:
        /** @brief Reads header from a .mat file.
         *  @return Number of bytes read.
         *  @param ec Set to \c std::errc::io_error if the header could not be read.
         */
        std::size_t read(std::ifstream& filestream, std::error_code& ec)
        {
            std::size_t int32_blocks_processed = 0;
            
            std::int32_t format_type_id = 0;
            std::int32_t height = 0;
            std::int32_t width = 0;
            std::int32_t complex_flag = 0;
            std::int32_t name_length = 0;
            char terminator = '\0';
            std::vector<char> text_data {};

            if (!filestream.is_open()) return this->on_read_error(ec, int32_blocks_processed);
            if (filestream.fail()) return this->on_read_error(ec, int32_blocks_processed);

            // Fixed-size portion of the header: metadata.
            filestream.read(reinterpret_cast<char*>(&format_type_id), sizeof(decltype(format_type_id))); // type
            if (filestream.fail()) return this->on_read_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.read(reinterpret_cast<char*>(&height), sizeof(decltype(height))); // mrows
            if (filestream.fail() || height < 0) return this->on_read_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.read(reinterpret_cast<char*>(&width), sizeof(decltype(width))); // ncols
            if (filestream.fail() || width < 0) return this->on_read_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.read(reinterpret_cast<char*>(&complex_flag), sizeof(decltype(complex_flag))); // imagf
            if (filestream.fail()) return this->on_read_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.read(reinterpret_cast<char*>(&name_length), sizeof(decltype(name_length))); // namlen
            if (filestream.fail() || name_length < 1) return this->on_read_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;

            // Variable-sized header: variable name.
            text_data.resize(name_length - 1);
            filestream.read(text_data.data(), name_length - 1);
            if (filestream.fail() || filestream.gcount() != name_length - 1) return this->on_read_error(ec, int32_blocks_processed);
            filestream.read(&terminator, 1);
            if (filestream.fail() || terminator != '\0') return this->on_read_error(ec, int32_blocks_processed);

            this->decompose_format_type_id(format_type_id);
            this->m_height = height;
            this->m_width = width;
            this->m_is_complex = (complex_flag == 0 ? false : true);
            this->m_name = std::string(text_data.begin(), text_data.end());

            return this->size();
        } // read(...)

        /** @brief Writes this header to \p filestream.
         *  @return Number of bytes written.
         *  @param ec Set to \c std::errc::io_error if the header could not be written.
         */
        std::size_t write(std::ofstream& filestream, std::error_code& ec) const
        {
            std::size_t int32_blocks_processed = 0;

            std::int32_t format_type_id = this->build_format_type_id();
            std::int32_t height = static_cast<std::int32_t>(this->m_height);
            std::int32_t width = static_cast<std::int32_t>(this->m_width);
            std::int32_t complex_flag = this->m_is_complex ? 1 : 0;
            std::int32_t name_length = static_cast<std::int32_t>(this->m_name.size() + 1);
            char terminator = '\0';

            if (!filestream.is_open()) return this->on_write_error(ec, int32_blocks_processed);
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);

            // Fixed-size portion of the header: metadata.
            filestream.write(reinterpret_cast<char*>(&format_type_id), sizeof(decltype(format_type_id))); // type
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.write(reinterpret_cast<char*>(&height), sizeof(decltype(height))); // mrows
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.write(reinterpret_cast<char*>(&width), sizeof(decltype(width))); // ncols
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.write(reinterpret_cast<char*>(&complex_flag), sizeof(decltype(complex_flag))); // imagf
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;
            filestream.write(reinterpret_cast<char*>(&name_length), sizeof(decltype(name_length))); // namlen
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            ++int32_blocks_processed;

            // Variable-sized header: variable name.
            filestream.write(this->m_name.c_str(), name_length - 1);
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);
            filestream.write(&terminator, 1);
            if (filestream.fail()) return this->on_write_error(ec, int32_blocks_processed);

            return this->size();
        } // write(...)

        /** Initializes the header for a given matrix. */
        template <typename t_value_type, typename t_allocator_type, typename t_arrangement_type>
        void initialize(
            const std::string& variable_name,
            const aftermath::algebra::matrix<t_value_type, t_allocator_type, t_arrangement_type>& mat,
            mat4_data_format data_format = mat4_data_format::ieee_little_endian, 
            mat4_matrix_type_id matrix_type_id = mat4_matrix_type_id::full) noexcept
        {
            using data_type = t_value_type;

            this->m_data_format_id = static_cast<std::int32_t>(data_format);
            this->m_data_type_id = mat4_data_type_id<data_type>::value;
            this->m_matrix_type_id = static_cast<std::int32_t>(matrix_type_id);

            this->m_height = static_cast<std::int32_t>(mat.height());
            this->m_width = static_cast<std::int32_t>(mat.width());

            this->m_name = variable_name;
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

        /** Gets the size, in bytes, of the current header. */
        std::size_t size() const noexcept
        {
            return 5 * sizeof(std::int32_t) + this->m_name.size() + 1;
        } // size(...)
    }; // struct mat4_header
} // namespace ropufu::aftermath::format

#endif // ROPUFU_AFTERMATH_FORMAT_MAT4_HEADER_HPP_INCLUDED
