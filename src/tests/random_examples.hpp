
#ifndef ROPUFU_TESTS_RANDOM_EXAMPLES_HPP_INCLUDED
#define ROPUFU_TESTS_RANDOM_EXAMPLES_HPP_INCLUDED

#include "../ropufu/probability.hpp"
#include "../ropufu/random.hpp"

#include <chrono>  // std::chrono::high_resolution_clock
#include <cstddef> // std::size_t
#include <cstdint> // std::int32_t
#include <random>  // std::seed_seq
#include <utility> // std::forward

namespace ropufu
{
    namespace test_aftermath
    {
        template <typename t_engine_type, typename t_param_type>
        struct binomial_benchmark
        {
            using type = binomial_benchmark<t_engine_type, t_param_type>;
            using engine_type = t_engine_type;
            using clock_type = std::chrono::steady_clock;
            using distribution_type = aftermath::probability::binomial_distribution<std::size_t, t_param_type>;
            using result_type = typename distribution_type::result_type;
            using param_type = typename distribution_type::param_type;
            using sampler_type = aftermath::random::sampler_binomial_alias<t_engine_type, std::size_t, t_param_type>;
            using lookup_sampler_type = aftermath::random::sampler_binomial_lookup<t_engine_type, std::size_t, t_param_type>;
            using builtin_distribution_type = std::binomial_distribution<std::size_t>;

        private:
            engine_type m_engine;
            std::size_t m_n_min, m_n_max;
            param_type m_probability_of_success;

            template <typename t_sampler_constructor_type>
            param_type compound_binomial(t_sampler_constructor_type& sampler_ctor, std::size_t m, param_type& elapsed_seconds) noexcept
            {
                auto tic = clock_type::now();
            
                std::uniform_int_distribution<std::size_t> uniform_n(this->m_n_min, this->m_n_max);
            
                std::size_t sum = 0;
                for (std::size_t i = 0; i < m; i++)
                {
                    std::size_t n = uniform_n(this->m_engine);
                    auto sampler = sampler_ctor(n, this->m_probability_of_success);
                    sum += sampler(this->m_engine);
                }
            
                auto toc = clock_type::now();
                elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count() / static_cast<param_type>(1'000);
                return static_cast<param_type>(sum) / m;
            }
            
            param_type compound_binomial_table(std::size_t m, param_type& elapsed_seconds) noexcept
            {
                auto tic = clock_type::now();
            
                std::uniform_int_distribution<std::size_t> uniform_n(this->m_n_min, this->m_n_max);
                
                lookup_sampler_type binomial_matrix(this->m_n_min, this->m_n_max, this->m_probability_of_success);
                //param_type size_in_megabytes = (binomial_matrix.size_in_bytes() / static_cast<param_type>(1024 * 1024));
            
                std::size_t sum = 0;
                for (std::size_t i = 0; i < m; i++)
                {
                    auto n = uniform_n(this->m_engine);
                    sum += binomial_matrix(n, this->m_engine);
                }
            
                auto toc = clock_type::now();
                elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count() / static_cast<param_type>(1'000);
                return static_cast<param_type>(sum) / m;
            }

        public:
            binomial_benchmark(std::size_t n_min, std::size_t n_max, param_type p) noexcept
                : m_engine(),
                m_n_min(n_min), m_n_max(n_max), m_probability_of_success(p)
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::seed_seq ss { 875, 393, 19, static_cast<std::int32_t>(now.time_since_epoch().count()) };
                this->m_engine.seed(ss);
            }
            
            void benchmark_compound(std::size_t m, param_type& elapsed_seconds_tested, param_type& elapsed_seconds_builtin) noexcept
            {
                auto builtin_ctor = [&](std::size_t n, param_type q) 
                {
                    return builtin_distribution_type(n, q);
                };
                
                this->compound_binomial_table(m, elapsed_seconds_tested);
                this->compound_binomial(builtin_ctor, m, elapsed_seconds_builtin);
            }
        };
    }
}

#endif // ROPUFU_TESTS_RANDOM_EXAMPLES_HPP_INCLUDED
