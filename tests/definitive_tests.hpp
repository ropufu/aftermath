
#ifndef ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED
#define ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED

#include "../aftermath/algebra.hpp"
#include "../aftermath/format.hpp"
#include "../aftermath/probability.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace ropufu
{
    namespace test_aftermath
    {
        struct format_test
        {
            typedef std::default_random_engine engine_type;
            typedef std::chrono::high_resolution_clock clock_type;
            typedef aftermath::format::matstream<4> matstream_type;
            //typedef aftermath::algebra::matrix_row_major<double> matrix_type;
            typedef aftermath::algebra::matrix<double, true> matrix_type;

        private:
            engine_type m_engine;
            std::string m_filename;

            std::vector<matrix_type> write_mat(std::size_t height, std::size_t width, std::size_t stack_size)
            {
                std::vector<matrix_type> matrices(0);
                matrices.reserve(stack_size);

                matstream_type mat(this->m_filename);
                mat.clear();
                
                std::uniform_real_distribution<double> uniform_real(0, 1);
                std::uniform_int_distribution<std::size_t> uniform_height(1, height);
                std::uniform_int_distribution<std::size_t> uniform_width(1, width);
                std::uniform_int_distribution<std::size_t> uniform_stack_size(1, stack_size);

                stack_size = uniform_stack_size(this->m_engine);
                for (std::size_t k = 0; k < stack_size; k++)
                {
                    std::size_t height_k = uniform_height(this->m_engine);
                    std::size_t width_k = uniform_width(this->m_engine);

                    matrix_type matrix(height_k, width_k);
                    for (std::size_t i = 0; i < height_k; i++) for (std::size_t j = 0; j < width_k; j++) matrix(i, j) = uniform_real(this->m_engine);
                    mat << "matrix" << k << matrix;

                    matrices.push_back(matrix);
                }
                return matrices;
            }

            bool read_mat(const std::vector<matrix_type>& matrices)
            {
                matstream_type mat(this->m_filename);
                for (const matrix_type& reference_matrix : matrices)
                {
                    matrix_type matrix;
                    std::string name = "";
                    mat.load(name, matrix);
                    if (matrix != reference_matrix) return false;
                }
                return true;
            }

        public:
            explicit format_test(const std::string& filename) noexcept
                : m_engine(clock_type::now().time_since_epoch().count()),
                m_filename(filename)
            {
            }
            
            bool test_matstream_v4(std::size_t m, std::size_t height, std::size_t width, std::size_t stack_size)
            {
                for (std::size_t i = 0; i < m; i++)
                {
                    std::vector<matrix_type>& matrices = this->write_mat(height, width, stack_size);
                    if (!this->read_mat(matrices)) return false;
                }
                return true;
            }
        };
    }
}

#endif // ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED
