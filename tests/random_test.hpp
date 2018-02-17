
#ifndef ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED
#define ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED

#include "../aftermath/not_an_error.hpp"
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
        namespace detail
        {
            template <typename t_distribution_type, typename t_engine_type>
            struct sampler_switch
            {
                using type = t_distribution_type; // Built-in C++ distributions are also samplers.
                using builtin_type = t_distribution_type; // Corresponding built-in C++ distribution type.
            };
            
            template <typename t_result_type, typename t_engine_type>
            struct sampler_switch<aftermath::probability::dist_normal<t_result_type>, t_engine_type>
            {
                using type = aftermath::random::default_sampler_normal_t<t_engine_type, t_result_type>;
                using builtin_type = std::normal_distribution<t_result_type>; // Corresponding built-in C++ distribution type.
            };
            
            template <typename t_result_type, typename t_engine_type>
            struct sampler_switch<aftermath::probability::dist_lognormal<t_result_type>, t_engine_type>
            {
                using type = aftermath::random::default_sampler_lognormal_t<t_engine_type, t_result_type>;
                using builtin_type = std::lognormal_distribution<t_result_type>; // Corresponding built-in C++ distribution type.
            };
        }

        template <typename t_distribution_type, typename t_engine_type>
        struct test_random
        {
            using type = test_random<t_distribution_type, t_engine_type>;
            using engine_type = t_engine_type;
            using clock_type = std::chrono::steady_clock;
            using distribution_type = t_distribution_type;
            using param_type = typename t_distribution_type::param_type;
            using sampler_type = typename detail::sampler_switch<t_distribution_type, t_engine_type>::type;
            using builtin_distribution_type = typename detail::sampler_switch<t_distribution_type, t_engine_type>::builtin_type;

        private:
            engine_type m_engine;
            distribution_type m_distribution;
            sampler_type m_sampler;
            builtin_distribution_type m_builtin_distribution;

            template <typename t_sampler_type>
            param_type tail_probability(t_sampler_type& sampler, std::size_t n, param_type tail, param_type& elapsed_seconds) noexcept
            {
                auto tic = clock_type::now();

                std::size_t count_tail = 0;
                for (std::size_t i = 0; i < n; i++) if (sampler(this->m_engine) > tail) count_tail++;

                auto toc = clock_type::now();
                elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count() / static_cast<param_type>(1'000);

                return static_cast<param_type>(count_tail) / n;
            }
            
            template <typename t_sampler_type>
            param_type cusum_run_length(t_sampler_type& sampler, std::size_t n, param_type threshold, param_type& elapsed_seconds) noexcept
            {
                auto tic = clock_type::now();

                param_type sum = 0;
                param_type drift = this->m_distribution.mean() + this->m_distribution.stddev() / 2;
                for (std::size_t i = 0; i < n; i++)
                {
                    std::size_t time = 0;
                    param_type cusum_statistic = 0;
                    while (cusum_statistic < threshold)
                    {
                        param_type z = sampler(this->m_engine) - drift;
                        cusum_statistic = (cusum_statistic < 0 ? 0 : cusum_statistic) + z;
                        time++;
                    }
                    sum += time;
                }

                auto toc = clock_type::now();
                elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count() / static_cast<param_type>(1'000);
                
                return sum / n;
            };

        public:
            template <typename ...t_args>
            test_random(t_args&&... args) noexcept
                : m_engine(),
                m_distribution(std::forward<t_args>(args)...),
                m_sampler(m_distribution),
                m_builtin_distribution(m_distribution.to_std())
            {
                auto now = std::chrono::high_resolution_clock::now();
                std::seed_seq ss = { 1, 7, 2, 9, static_cast<std::int_fast32_t>(now.time_since_epoch().count()) };
                this->m_engine.seed(ss);
            }
            
            void benchmark_tail(std::size_t n, param_type tail, param_type& elapsed_seconds_tested, param_type& elapsed_seconds_builtin) noexcept
            {
                this->tail_probability(this->m_sampler, n, tail, elapsed_seconds_tested);
                this->tail_probability(this->m_builtin_distribution, n, tail, elapsed_seconds_builtin);
            }
            
            param_type error_in_tail(std::size_t n, param_type tail) noexcept
            {
                param_type you_big_dummy = 0;
                auto x_test = this->tail_probability(this->m_sampler, n, tail, you_big_dummy);

                param_type p_reference = 1 - this->m_distribution.cdf(tail);
                param_type diff = (x_test - p_reference) / p_reference;
                if (diff < 0) diff = -diff;
    
                return diff;
            }
            
            void benchmark_cusum(std::size_t n, param_type threshold, param_type& elapsed_seconds_tested, param_type& elapsed_seconds_builtin) noexcept
            {
                this->cusum_run_length(this->m_sampler, n, threshold, elapsed_seconds_tested);
                this->cusum_run_length(this->m_builtin_distribution, n, threshold, elapsed_seconds_builtin);
            }
            
            param_type error_in_cusum(std::size_t n, param_type threshold) noexcept
            {
                param_type you_big_dummy = 0;
                auto x_test = this->cusum_run_length(this->m_sampler, n, threshold, you_big_dummy);
                auto x_builtin = this->cusum_run_length(this->m_builtin_distribution, n, threshold, you_big_dummy);
                
                param_type diff = (x_test - x_builtin) / x_builtin;
                if (diff < 0) diff = -diff;
    
                return diff;
            }
        };
    }
}

#endif // ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED
