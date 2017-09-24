
#ifndef ROPUFU_TESTS_RANDOM_EXAMPLES_HPP_INCLUDED
#define ROPUFU_TESTS_RANDOM_EXAMPLES_HPP_INCLUDED

#include "../aftermath/probability.hpp"
#include "../aftermath/random.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>  // For std::forward.

namespace ropufu
{
    namespace test_aftermath
    {
        template <typename t_engine_type = std::default_random_engine>
        struct binomial_benchmark
        {
            typedef binomial_benchmark<t_engine_type> type;
            typedef t_engine_type engine_type;
            typedef std::chrono::high_resolution_clock clock_type;
            typedef aftermath::probability::dist_binomial distribution_type;
            typedef aftermath::random::default_sampler_binomial_t<engine_type> sampler_type;
            typedef aftermath::random::default_sampler_binomial_lookup_t<engine_type> lookup_sampler_type;
            typedef std::binomial_distribution<std::size_t> builtin_distribution_type;

        private:
            engine_type m_engine;
            std::size_t m_n_min, m_n_max;
            double m_probability_of_success;

            template <typename t_sampler_constructor_type>
            double compound_binomial(t_sampler_constructor_type& sampler_ctor, std::size_t m, double& elapsed_time)
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
                auto dt = toc.time_since_epoch() - tic.time_since_epoch();
                elapsed_time = static_cast<double>(dt.count() * clock_type::period::num) / clock_type::period::den;
                return static_cast<double>(sum) / m;
            }
            
            double compound_binomial_table(std::size_t m, double& elapsed_time)
            {
                auto tic = clock_type::now();
            
                std::uniform_int_distribution<std::size_t> uniform_n(this->m_n_min, this->m_n_max);
                
                distribution_type from(this->m_n_min, this->m_probability_of_success);
                distribution_type to(this->m_n_max, this->m_probability_of_success);
                lookup_sampler_type binomial_matrix(from, to);
                //double size_in_megabytes = (binomial_matrix.size_in_bytes() / (1024.0 * 1024.0));
            
                std::size_t sum = 0;
                for (std::size_t i = 0; i < m; i++)
                {
                    auto n = uniform_n(this->m_engine);
                    sum += binomial_matrix(n, this->m_engine);
                }
            
                auto toc = clock_type::now();
                auto dt = toc.time_since_epoch() - tic.time_since_epoch();
                elapsed_time = static_cast<double>(dt.count() * clock_type::period::num) / clock_type::period::den;
                return static_cast<double>(sum) / m;
            }

        public:
            binomial_benchmark(std::size_t n_min, std::size_t n_max, double p)
                : m_engine(clock_type::now().time_since_epoch().count()),
                m_n_min(n_min), m_n_max(n_max), m_probability_of_success(p)
            {
            }
            
            void benchmark_compound(std::size_t m, double& elapsed_seconds_tested, double& elapsed_seconds_builtin)
            {
                auto builtin_ctor = [&](std::size_t n, double q) 
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
