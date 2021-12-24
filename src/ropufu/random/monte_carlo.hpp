
#ifndef ROPUFU_AFTERMATH_RANDOM_MONTE_CARLO_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_MONTE_CARLO_HPP_INCLUDED

#include <array>       // std::array
#include <concepts>    // std::default_initializable
#include <cstddef>     // std::size_t, std::nullptr_t
#include <future>      // std::promise, std::future
#include <stdexcept>   // std::logic_error
#include <stop_token>  // std::stop_source, std::stop_token
#include <thread>      // std::jthread
#include <utility>     // std::move

namespace ropufu::aftermath::random
{
    template <typename t_simulator_type>
    concept monte_carlo_simulator = std::default_initializable<t_simulator_type> &&
        requires(t_simulator_type& f)
        {
            {f()};
        }; // concept monte_carlo_simulator

    template <typename t_aggregator_type, typename t_simulator_type>
    concept monte_carlo_aggregator =
        std::default_initializable<t_aggregator_type> &&
        monte_carlo_simulator<t_simulator_type> &&
        requires(t_aggregator_type& g, t_simulator_type& f, const t_aggregator_type& h)
        {
            {g(f())};
            {g(h)};
        }; // concept monte_carlo_aggregator

    template <monte_carlo_simulator t_simulator_type,
        monte_carlo_aggregator<t_simulator_type> t_aggregator_type,
        std::size_t t_count_threads = 1>
        requires (t_count_threads > 0)
    struct monte_carlo;

    template <typename t_aggregator_type>
    struct monte_carlo_task
    {
        using type = monte_carlo_task<t_aggregator_type>;
        using aggregator_type = t_aggregator_type;

        template <monte_carlo_simulator t_simulator_type, monte_carlo_aggregator<t_simulator_type>, std::size_t t_count_threads>
            requires (t_count_threads > 0)
        friend struct monte_carlo;

    private:
        std::promise<aggregator_type> m_promise = {};
        std::future<aggregator_type> m_future = {};
        std::stop_source m_cancellation_source = {};

        std::stop_token get_token() const noexcept
        {
            return this->m_cancellation_source.get_token();
        } // get_token(...)

        void set_value(const aggregator_type& result)
        {
            this->m_promise.set_value(result);
        } // set_value(...)

        /** Invalid task. */
        monte_carlo_task(std::nullptr_t) noexcept
        {
        } // monte_carlo_task(...)

        /** Default task. */
        monte_carlo_task() noexcept
            : m_promise(), m_future(this->m_promise.get_future())
        {
        } // monte_carlo_task(...)

        monte_carlo_task(const type&) = delete;

        monte_carlo_task(type&& other) noexcept
        {
            this->m_promise = std::move(other.m_promise);
            this->m_future = std::move(other.m_future);
            this->m_cancellation_source = std::move(other.m_cancellation_source);
        } // monte_carlo_task(...)

        void operator =(type&& other) noexcept
        {
            this->m_promise = std::move(other.m_promise);
            this->m_future = std::move(other.m_future);
            this->m_cancellation_source = std::move(other.m_cancellation_source);
        } // operator =(...)

    public:
        bool valid() const noexcept
        {
            return this->m_future.valid();
        } // valid(...)

        // Requests to terminate the simulation.
        void request_stop() noexcept
        {
            this->m_cancellation_source.request_stop();
        } // request_stop(...)

        // Waits for the simulation to complete execution.
        aggregator_type wait()
        {
            return this->m_future.get();
        } // wait(...)
    }; // struct monte_carlo_task

    template <monte_carlo_simulator t_simulator_type,
        monte_carlo_aggregator<t_simulator_type> t_aggregator_type,
        std::size_t t_count_threads>
        requires (t_count_threads > 0)
    struct monte_carlo
    {
        using type = monte_carlo<t_simulator_type, t_aggregator_type, t_count_threads>;
        using simulator_type = t_simulator_type;
        using aggregator_type = t_aggregator_type;

        using task_type = monte_carlo_task<aggregator_type>;

        static constexpr std::size_t count_threads = t_count_threads;

    private:
        task_type m_execution_task = task_type(nullptr);
        std::array<simulator_type, count_threads> m_simulators = {};
        std::array<std::jthread, count_threads> m_simulator_threads = {};

        static std::array<std::size_t, t_count_threads> simulations_per_thread(std::size_t count_simulations) noexcept
        {
            std::array<std::size_t, count_threads> result {};
            std::size_t n = count_simulations / count_threads;
            std::size_t k = count_simulations % count_threads;
            // count_simulations = n * count_threads + k.
            result.fill(n);
            for (std::size_t i = 0; i < k; ++i) result[i] = n + 1;
            return result;
        } // simulations_per_thread(...)

        aggregator_type execute(std::size_t count_simulations, std::stop_token token) noexcept
        {
            std::array<std::size_t, count_threads> simulation_counts = type::simulations_per_thread(count_simulations);
            std::array<aggregator_type, count_threads> aggregators{};

            for (std::size_t k = 0; k < count_threads; ++k)
            {
                std::size_t m = simulation_counts[k];
                simulator_type& simulator = this->m_simulators[k];
                aggregator_type& aggregator = aggregators[k];
                this->m_simulator_threads[k] = std::jthread(
                    [m, &simulator, &aggregator](std::stop_token token) {
                        for (std::size_t i = 0; i < m; ++i)
                        {
                            aggregator(simulator());
                            if (token.stop_requested()) break;
                        } // for (...)
                    }, token);
            } // for (...)

            for (std::jthread& x : this->m_simulator_threads) x.join();
            aggregator_type joint_aggregator{};
            for (const aggregator_type& x : aggregators) joint_aggregator(x);
            return joint_aggregator;
        } // execute(...)

    public:
        monte_carlo() noexcept = default;

        explicit monte_carlo(const std::array<simulator_type, count_threads>& simulators) noexcept
            : m_simulators(simulators)
        {
        } // monte_carlo(...)

        monte_carlo(const type&) = delete;

        monte_carlo(type&&) = delete;

        aggregator_type execute_sync(std::size_t count_simulations)
        {
            this->begin_async(count_simulations);
            return this->m_execution_task.wait();
        } // execute_sync(...)

        task_type& begin_async(std::size_t count_simulations)
        {
            if (this->m_execution_task.valid()) throw std::logic_error("Monte carlo simulations still running.");
            this->m_execution_task = task_type();

            std::jthread t(
                [this, count_simulations]() {
                    aggregator_type result = this->execute(count_simulations, this->m_execution_task.get_token());
                    this->m_execution_task.set_value(result);
                });
            t.detach();

            return this->m_execution_task;
        } // begin_async(...)
    }; // struct monte_carlo
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_MONTE_CARLO_HPP_INCLUDED
