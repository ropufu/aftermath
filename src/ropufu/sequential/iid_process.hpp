
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED

#include "scalar_process.hpp"

#include <chrono>  // std::chrono::system_clock
#include <cstddef> // std::size_t
#include <random>  // std::seed_seq
#include <ranges>  // std::ranges::...

namespace ropufu::aftermath::sequential
{
    template <typename t_engine_type, typename t_sampler_type>
        requires std::totally_ordered<typename t_sampler_type::value_type>
    struct iid_process : public scalar_process<typename t_sampler_type::value_type>
    {
        using type = iid_process<t_engine_type, t_sampler_type>;
        using base_type = scalar_process<typename t_sampler_type::value_type>;
        using engine_type = t_engine_type;
        using sampler_type = t_sampler_type;

	    using distribution_type = typename sampler_type::distribution_type;
        using value_type = typename sampler_type::value_type;

    private:
        engine_type m_engine;
        sampler_type m_sampler;

        static engine_type make_engine() noexcept
        {
            engine_type result{};
            int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
            std::seed_seq sequence{ 1, 1, 2, 3, 5, 8, 1729, time_seed };
            result.seed(sequence);
            return result;
        } // make_engine(...)

    protected:
        void on_clear() noexcept override { }

        value_type on_next() noexcept override
        {
            return this->m_sampler(this->m_engine);
        } // next(...)

        template <std::ranges::random_access_range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        virtual void on_next(t_container_type& values) noexcept override
        {
            for (value_type& x : values) x = this->m_sampler(this->m_engine);
        } // on_next(...)

    public:
        explicit iid_process(const distribution_type& dist) noexcept
            : m_engine(type::make_engine()), m_sampler(dist)
        {
        } // iid_process(...)
    }; // struct iid_process
} // namespace ropufu::sequential::intermittent

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_IID_PROCESS_HPP_INCLUDED
