
#ifndef ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_SAMPLER_HPP_INCLUDED

#include "../math_constants.hpp"
#include "../probability/distribution_traits.hpp"

#include <cstddef>     // std:size_t
#include <limits>      // std::numeric_limits
#include <type_traits> // std::is_same_v

namespace ropufu::aftermath::random
{
    /** @brief Base class for ziggurat pseudo-random number generators. */
    template <typename t_derived_type, typename t_engine_type, typename t_distribution_type, std::size_t t_n_boxes>
    struct ziggurat_sampler
    {
        using type = ziggurat_sampler<t_derived_type, t_engine_type, t_distribution_type, t_n_boxes>;
        using derived_type = t_derived_type;
        using engine_type = t_engine_type;
        using distribution_type = t_distribution_type;
        using value_type = typename distribution_type::value_type;
        using probability_type = typename distribution_type::probability_type;
        using expectation_type = typename distribution_type::expectation_type;
        using uniform_type = typename t_engine_type::result_type;

        using result_type = value_type;
        static constexpr std::size_t n_boxes = t_n_boxes;
        static constexpr uniform_type engine_diameter = engine_type::max() - engine_type::min();

        static constexpr bool has_left_tail = aftermath::probability::has_left_tail_v<distribution_type>;
        static constexpr bool has_right_tail = aftermath::probability::has_right_tail_v<distribution_type>;

    private:
        distribution_type m_distribution = {};

        static constexpr void traits_check() noexcept
        {
            static_assert(n_boxes > 1, "Number of boxes has to be at least two.");
            static_assert(aftermath::is_power_of_two(n_boxes), "Number of boxes has to be a power of two.");
            static_assert(std::numeric_limits<uniform_type>::is_integer, "Engine result type has to be an integer type.");
        } // traits_check(...)

    protected:
        // static const uniform_type upscaled_low_probabilities[n_boxes];
        // static const uniform_type upscaled_high_probabilities[n_boxes];
        // static const value_type layer_left_endpoints[n_boxes];
        // static const value_type downscaled_layer_widths[n_boxes];
        // static const expectation_type layer_bottom_endpoints[n_boxes];
        // static const expectation_type downscaled_layer_heights[n_boxes];

        value_type sample_left_tail(engine_type& uniform_generator) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::sample_left_tail), 
                decltype(&type::sample_left_tail)>;
            static_assert(is_overwritten, "sample_left_tail(engine_type&) noexcept -> value_type was not overwritten.");
            derived_type* that = static_cast<derived_type*>(this);
            return that->sample_left_tail(uniform_generator);
        } // sample_left_tail(...)

        value_type sample_right_tail(engine_type& uniform_generator) noexcept
        {
            constexpr bool is_overwritten = !std::is_same_v<
                decltype(&derived_type::sample_right_tail), 
                decltype(&type::sample_right_tail)>;
            static_assert(is_overwritten, "sample_right_tail(engine_type&) noexcept -> value_type was not overwritten.");
            derived_type* that = static_cast<derived_type*>(this);
            return that->sample_right_tail(uniform_generator);
        } // sample_right_tail(...)

    public:
        ziggurat_sampler() noexcept { type::traits_check(); }

        /*implicit*/ ziggurat_sampler(const distribution_type& distribution) noexcept
            : m_distribution(distribution)
        {
            type::traits_check();
        } // ziggurat_sampler(...)

        const distribution_type& distribution() const noexcept { return this->m_distribution; }

        /** @todo Think about inequalities (strict vs. non-strict) in the \c u2 comparisons. */
        value_type sample(engine_type& uniform_generator) noexcept
        {
            constexpr uniform_type layer_mask = static_cast<uniform_type>(n_boxes - 1);
            while (true)
            {
                uniform_type u1 = uniform_generator() - engine_type::min(); // Layer index.
                uniform_type u2 = uniform_generator() - engine_type::min(); // Horizontal component.

                uniform_type layer_index = static_cast<uniform_type>(u1 & layer_mask); // Pick a layer.

                if (layer_index == 0) // Bottom layer.
                {
                    if constexpr (type::has_left_tail)
                    {
                        if (u2 < derived_type::upscaled_low_probabilities[0]) return this->sample_left_tail(uniform_generator);
                    } // if constexpr (...)
                    if constexpr (type::has_right_tail)
                    {
                        if (u2 > derived_type::upscaled_high_probabilities[0]) return this->sample_right_tail(uniform_generator);
                    } // if constexpr (...)
                    return derived_type::layer_left_endpoints[0] + u2 * derived_type::downscaled_layer_widths[0];
                } // if (...)
                else
                {
                    value_type x = derived_type::layer_left_endpoints[layer_index] + u2 * derived_type::downscaled_layer_widths[layer_index];
                    bool is_interior = true;
                    if constexpr (type::has_left_tail)
                    {
                        if (u2 <= derived_type::upscaled_low_probabilities[layer_index]) is_interior = false;
                    } // if constexpr (...)
                    if constexpr (type::has_right_tail)
                    {
                        if (u2 >= derived_type::upscaled_high_probabilities[layer_index]) is_interior = false;
                    } // if constexpr (...)
                    if (is_interior) return x;

                    uniform_type u3 = uniform_generator() - engine_type::min(); // Vertical component.
                    if (derived_type::layer_bottom_endpoints[layer_index] + u3 * derived_type::downscaled_layer_heights[layer_index] < this->m_distribution.pdf(x)) return x;
                } // else (...)
            } // while(...)
        } // sample(...)

        value_type operator ()(engine_type& uniform_generator) noexcept
        {
            return this->sample(uniform_generator);
        } // operator ()(...)
    }; // struct ziggurat
} // namespace ropufu::aftermath::random

#endif // ROPUFU_AFTERMATH_RANDOM_ZIGGURAT_SAMPLER_HPP_INCLUDED
