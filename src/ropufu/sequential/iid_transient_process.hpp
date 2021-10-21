
#ifndef ROPUFU_AFTERMATH_SEQUENTIAL_IID_TRANSIENT_PROCESS_HPP_INCLUDED
#define ROPUFU_AFTERMATH_SEQUENTIAL_IID_TRANSIENT_PROCESS_HPP_INCLUDED

#include "scalar_process.hpp"

#include <chrono>  // std::chrono::system_clock
#include <cstddef> // std::size_t
#include <random>  // std::seed_seq
#include <ranges>  // std::ranges::...
#include <stdexcept> // std::logic_error

namespace ropufu::aftermath::sequential
{
    template <typename t_engine_type, typename t_no_change_sampler_type, typename t_under_change_sampler_type>
        requires
            std::totally_ordered<typename t_no_change_sampler_type::value_type> &&
            std::same_as<typename t_no_change_sampler_type::value_type, typename t_under_change_sampler_type::value_type>
    struct iid_transient_process : public scalar_process<typename t_no_change_sampler_type::value_type>
    {
        using type = iid_transient_process<t_engine_type, t_no_change_sampler_type, t_under_change_sampler_type>;
        using base_type = scalar_process<typename t_no_change_sampler_type::value_type>;
        using engine_type = t_engine_type;
        using no_change_sampler_type = t_no_change_sampler_type;
        using under_change_sampler_type = t_under_change_sampler_type;

	    using no_change_distribution_type = typename no_change_sampler_type::distribution_type;
	    using under_change_distribution_type = typename under_change_sampler_type::distribution_type;
        using value_type = typename no_change_sampler_type::value_type;

    private:
        engine_type m_engine;
        no_change_sampler_type m_no_change_sampler;
        under_change_sampler_type m_under_change_sampler;
        std::size_t m_first_under_change_index;
        std::size_t m_last_under_change_index;

        static engine_type make_engine() noexcept
        {
            engine_type result{};
            int time_seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());
            std::seed_seq sequence{ 0, 1, 3, 6, 10, 15, 1729, time_seed };
            result.seed(sequence);
            return result;
        } // make_engine(...)

    protected:
        void on_clear() noexcept override { }

        value_type on_next() noexcept override
        {
            std::size_t count = this->count();
            return (count >= this->m_first_under_change_index && count <= this->m_last_under_change_index) ?
                this->m_under_change_sampler(this->m_engine) : this->m_no_change_sampler(this->m_engine);
        } // on_next(...)

        template <std::ranges::random_access_range t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        virtual void on_next(t_container_type& values) noexcept override
        {
            std::size_t count = this->count();
            std::size_t length = std::ranges::size(values);
            // @todo Consider hashing the change passed/in progress flags.
            if (count > this->m_last_under_change_index) // Change has passed.
            {
                for (value_type& x : values) x = this->m_no_change_sampler(this->m_engine);
            } // if (...)
            else if (count + length <= this->m_first_under_change_index) // Change is in the distant future.
            {
                for (value_type& x : values) x = this->m_under_change_sampler(this->m_engine);
            } // if (...)
            else
            {
                // Three blocks: pre-change, under-change, post-change.
                std::size_t pre_size = (this->m_first_under_change_index > count) ? this->m_first_under_change_index - count : 0;
                std::size_t post_size = (count + length > this->m_last_under_change_index + 1) ? count + length - this->m_last_under_change_index - 1 : 0;
                std::size_t under_size = length - pre_size - post_size;
                std::size_t pre_under_size = pre_size + under_size;
                for (std::size_t i = 0; i < pre_size; ++i) values[i] = this->m_no_change_sampler(this->m_engine);
                for (std::size_t i = pre_size; i < pre_under_size; ++i) values[i] = this->m_under_change_sampler(this->m_engine);
                for (std::size_t i = pre_under_size; i < length; ++i) values[i] = this->m_no_change_sampler(this->m_engine);
            } // else
        } // on_next(...)

    public:
        iid_transient_process(const no_change_distribution_type& no_change_dist, const under_change_distribution_type& under_change_dist,
            std::size_t first_under_change_index, std::size_t change_duration)
            : m_engine(type::make_engine()), m_no_change_sampler(no_change_dist), m_under_change_sampler(under_change_dist),
            m_first_under_change_index(first_under_change_index), m_last_under_change_index(first_under_change_index + change_duration - 1)
        {
            if (change_duration == 0) throw std::logic_error("Change duration cannot be zero.");
        } // iid_transient_process(...)
    }; // struct iid_transient_process
} // namespace ropufu::sequential::intermittent

#endif // ROPUFU_AFTERMATH_SEQUENTIAL_IID_TRANSIENT_PROCESS_HPP_INCLUDED
