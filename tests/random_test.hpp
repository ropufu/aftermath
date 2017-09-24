
#ifndef ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED
#define ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED

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
            template <typename t_distribution_type>
            struct sampler_switch
            {
                typedef t_distribution_type type; // Built-in C++ distributions are also samplers.
                typedef t_distribution_type builtin_type; // Corresponding built-in C++ distribution type.
            };
            
            template <>
            struct sampler_switch<aftermath::probability::dist_normal>
            {
                typedef aftermath::random::default_sampler_normal_t<std::default_random_engine> type;
                typedef std::normal_distribution<double> builtin_type; // Corresponding built-in C++ distribution type.
            };
            
            template <>
            struct sampler_switch<aftermath::probability::dist_lognormal>
            {
                typedef aftermath::random::default_sampler_lognormal_t<std::default_random_engine> type;
                typedef std::lognormal_distribution<double> builtin_type; // Corresponding built-in C++ distribution type.
            };
        }

        template <typename t_distribution_type, typename t_engine_type = std::default_random_engine>
        struct test_random
        {
            typedef test_random<t_distribution_type, t_engine_type> type;
            typedef t_engine_type engine_type;
            typedef std::chrono::high_resolution_clock clock_type;
            typedef t_distribution_type distribution_type;
            typedef typename detail::sampler_switch<distribution_type>::type sampler_type;
            typedef typename detail::sampler_switch<distribution_type>::builtin_type builtin_distribution_type;

        private:
            engine_type m_engine;
            distribution_type m_distribution;
            sampler_type m_sampler;
            builtin_distribution_type m_builtin_distribution;

            template <typename t_sampler_type>
            double tail_probability(t_sampler_type& sampler, std::size_t n, double tail, double& elapsed_time) noexcept
            {
                auto tic = clock_type::now();

                std::size_t count_tail = 0;
                for (std::size_t i = 0; i < n; i++) if (sampler(this->m_engine) > tail) count_tail++;

                auto toc = clock_type::now();
                auto dt = toc.time_since_epoch() - tic.time_since_epoch();
                elapsed_time = static_cast<double>(dt.count() * clock_type::period::num) / clock_type::period::den;

                return static_cast<double>(count_tail) / n;
            }
            
            template <typename t_sampler_type>
            double cusum_run_length(t_sampler_type& sampler, std::size_t n, double threshold, double& elapsed_time) noexcept
            {
                auto tic = clock_type::now();

                double sum = 0;
                double drift = this->m_distribution.mean() + this->m_distribution.stddev() / 2;
                for (std::size_t i = 0; i < n; i++)
                {
                    std::size_t time = 0;
                    double cusum_statistic = 0;
                    while (cusum_statistic < threshold)
                    {
                        double z = sampler(this->m_engine) - drift;
                        cusum_statistic = (cusum_statistic < 0 ? 0 : cusum_statistic) + z;
                        time++;
                    }
                    sum += time;
                }

                auto toc = clock_type::now();
                auto dt = toc.time_since_epoch() - tic.time_since_epoch();
                elapsed_time = static_cast<double>(dt.count() * clock_type::period::num) / clock_type::period::den;
                
                return sum / n;
            };

        public:
            template <typename ...t_args>
            test_random(t_args&&... args)
                : m_engine(clock_type::now().time_since_epoch().count()),
                m_distribution(std::forward<t_args>(args)...),
                m_sampler(m_distribution),
                m_builtin_distribution(m_distribution.to_std())
            {
            }
            
            void benchmark_tail(std::size_t n, double tail, double& elapsed_seconds_tested, double& elapsed_seconds_builtin)
            {
                this->tail_probability(this->m_sampler, n, tail, elapsed_seconds_tested);
                this->tail_probability(this->m_builtin_distribution, n, tail, elapsed_seconds_builtin);
            }
            
            double error_in_tail(std::size_t n, double tail)
            {
                double you_big_dummy = 0;
                auto x_test = this->tail_probability(this->m_sampler, n, tail, you_big_dummy);

                double p_reference = 1 - this->m_distribution.cdf(tail);
                double diff = (x_test - p_reference) / p_reference;
                if (diff < 0) diff = -diff;
    
                return diff;
            }
            
            void benchmark_cusum(std::size_t n, double threshold, double& elapsed_seconds_tested, double& elapsed_seconds_builtin)
            {
                this->cusum_run_length(this->m_sampler, n, threshold, elapsed_seconds_tested);
                this->cusum_run_length(this->m_builtin_distribution, n, threshold, elapsed_seconds_builtin);
            }
            
            double error_in_cusum(std::size_t n, double threshold)
            {
                double you_big_dummy = 0;
                auto x_test = this->cusum_run_length(this->m_sampler, n, threshold, you_big_dummy);
                auto x_builtin = this->cusum_run_length(this->m_builtin_distribution, n, threshold, you_big_dummy);
                
                double diff = (x_test - x_builtin) / x_builtin;
                if (diff < 0) diff = -diff;
    
                return diff;
            }
        };
    }
}

#endif // ROPUFU_TESTS_RANDOM_TEST_HPP_INCLUDED
