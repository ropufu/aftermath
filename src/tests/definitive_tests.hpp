
#ifndef ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED
#define ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED

#include "../ropufu/algebra.hpp"
#include "../ropufu/format.hpp"
#include "../ropufu/probability.hpp"

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
            using engine_type = std::default_random_engine;
            using matstream_type = aftermath::format::matstream<4>;
            using matrix_type = aftermath::algebra::matrix<double>;

        private:
            std::string m_filename;

            std::vector<matrix_type> write_mat(std::size_t height, std::size_t width, std::size_t stack_size) noexcept
            {
                std::seed_seq ss = {1, 7, 2, 9};
                engine_type engine(ss);

                std::vector<matrix_type> matrices(0);
                matrices.reserve(stack_size);

                matstream_type mat(this->m_filename);
                mat.clear();
                
                std::uniform_real_distribution<double> uniform_real(0, 1);
                std::uniform_int_distribution<std::size_t> uniform_height(1, height);
                std::uniform_int_distribution<std::size_t> uniform_width(1, width);
                std::uniform_int_distribution<std::size_t> uniform_stack_size(1, stack_size);

                stack_size = uniform_stack_size(engine);
                for (std::size_t k = 0; k < stack_size; k++)
                {
                    std::size_t height_k = uniform_height(engine);
                    std::size_t width_k = uniform_width(engine);

                    matrix_type matrix(height_k, width_k);
                    for (std::size_t i = 0; i < height_k; i++) for (std::size_t j = 0; j < width_k; j++) matrix.at(i, j) = uniform_real(engine);
                    mat << "matrix" << k << matrix;

                    matrices.push_back(matrix);
                }
                return matrices;
            }

            bool check_read_mat(const std::vector<matrix_type>& matrices) noexcept
            {
                matstream_type mat(this->m_filename);
                for (const matrix_type& reference_matrix : matrices)
                {
                    matrix_type matrix;
                    std::string name = "";
                    mat.load(name, matrix);
                    
                    if (matrix != reference_matrix) return false;
                }
                mat.clear();
                return true;
            }

        public:
            explicit format_test(const std::string& filename) noexcept
                : m_filename(filename)
            {
            }
            
            bool test_matstream_v4(std::size_t m, std::size_t height, std::size_t width, std::size_t stack_size) noexcept
            {
                for (std::size_t i = 0; i < m; i++)
                {
                    std::vector<matrix_type> matrices = this->write_mat(height, width, stack_size);
                    if (!this->check_read_mat(matrices)) return false;
                }
                return true;
            }
        };
    }
}

#endif // ROPUFU_TESTS_FORMAT_TEST_HPP_INCLUDED
