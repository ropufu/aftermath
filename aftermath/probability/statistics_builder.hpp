
#ifndef ROPUFU_AFTERMATH_PROBABILITY_STATISTICS_BUILDER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_STATISTICS_BUILDER_HPP_INCLUDED

#include "../type_concepts.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            namespace detail
            {
                /** Statistic to keep track of minimum and maximum elements in an online fashion. */
                template <typename t_key_type, typename t_probability_type,
                    bool t_is_enabled = type_impl::has_less<t_key_type>::value>
                struct order_statistic
                {
                    static constexpr bool is_enabled = t_is_enabled;

                private:
                    bool m_initialized = false;
                    t_key_type m_min, m_max;

                public:
                    /** Clears the statistic. */
                    void clear() noexcept
                    {
                        this->m_initialized = false;
                    }

                    /** Observe another element. */
                    void observe(const t_key_type& key, bool& is_new_min, bool& is_new_max) noexcept
                    {
                        if (!this->m_initialized)
                        {
                            this->m_min = key;
                            this->m_max = key;
                            is_new_min = true;
                            is_new_max = true;
                            this->m_initialized = true;
                        }
                        else if (key < this->m_min)
                        {
                            this->m_min = key;
                            is_new_min = true;
                        }
                        else if (this->m_max < key)
                        {
                            this->m_max = key;
                            is_new_max = true;
                        }
                    }

                    /** Smallest observed key. */
                    const t_key_type& min() const { return this->m_min; }
                    /** Largest observed key. */
                    const t_key_type& max() const { return this->m_max; }

                    /** Compute empirical cumulative distribution function (c.d.f.). */
                    template <typename t_dictionary_type>
                    t_probability_type cdf(const t_key_type& key, typename t_dictionary_type::mapped_type norm, const t_dictionary_type& data) const noexcept
                    {
                        if (key < this->m_min) return 0.0;
                        if (!(key < this->m_max)) return 1.0;

                        using count_type = typename t_dictionary_type::mapped_type;

                        count_type cumulative_count = count_type(); // zero
                        for (const auto& item : data)
                        {
                            if (key < item.first) break;
                            cumulative_count += item.second;
                        }
                        return cumulative_count / static_cast<t_probability_type>(norm);
                    }

                    /** Compute empirical percentile. */
                    template <typename t_dictionary_type>
                    const t_key_type& percentile(t_probability_type probability, typename t_dictionary_type::mapped_type norm, const t_dictionary_type& data) const noexcept
                    {
                        //if (probability < 0.0) cresult = std::error_condition(-1, std::out_of_range);
                        //if (probability > 1.0) cresult = std::error_condition(1, std::out_of_range);  
                        //cresult.clear();

                        if (probability <= 0.0) return this->m_min;
                        if (probability >= 1.0) return this->m_max;

                        using count_type = typename t_dictionary_type::mapped_type;

                        count_type p = count_type(); // zero
                        // Think in counts rather than probabilities.
                        probability *= norm;
                        count_type threshold = static_cast<count_type>(probability);
                        if (std::is_integral<count_type>::value && threshold < probability) threshold++; // For integral types: we need a ceiling, not floor.

                        for (const auto& item : data)
                        {
                            p += item.second;
                            if (p >= threshold) return item.first;
                        }
                        return this->m_max;
                    }
                };

                /** When not supported, do nothing. */
                template <typename t_key_type, typename t_probability_type>
                struct order_statistic<t_key_type, t_probability_type, false>
                {
                    static constexpr bool is_enabled = false;

                    void clear() const noexcept { }
                    void observe(const t_key_type&, bool&, bool&) const noexcept { }
                };

                /** Statistic to keep track of sum and mean in an online fashion. */
                template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type,
                    bool t_is_enabled = 
                        type_impl::has_product<t_key_type, t_count_type>::value &&
                        type_impl::has_sum<t_sum_type, t_key_type>::value &&
                        type_impl::has_divide<t_mean_type, t_count_type>::value>
                struct linear_statistic
                {
                    static constexpr bool is_enabled = t_is_enabled;

                private:
                    t_sum_type m_sum = t_sum_type();

                public:
                    /** Clears the statistic. */
                    void clear() noexcept
                    {
                        this->m_sum = t_sum_type();
                    }

                    /** Observe another element. */
                    void observe(const t_key_type& key, t_count_type repeat) noexcept
                    {
                        this->m_sum += static_cast<t_sum_type>(key * repeat);
                    }

                    /** Sum of the observations. */
                    const t_sum_type& sum() const noexcept
                    {
                        return this->m_sum;
                    }

                    /** Mean of the observations. */
                    t_mean_type mean(t_count_type norm) const
                    {
                        return static_cast<t_mean_type>(this->m_sum) / norm;
                    }
                };

                /** When not supported, do nothing. */
                template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
                struct linear_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type, false>
                {
                    static constexpr bool is_enabled = false;
                    
                    void clear() const noexcept { }
                    void observe(const t_key_type&, std::size_t) const noexcept { }
                };

                /** Statistic to keep track of variance in an offline fashion. */
                template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type,
                    bool t_is_enabled =
                        linear_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type>::is_enabled &&
                        type_impl::has_difference<t_key_type, t_mean_type>::value &&
                        std::is_scalar<typename type_impl::get_product_result<t_key_type, t_key_type>::type>::value>
                struct variance_statistic
                {
                    static constexpr bool is_enabled = t_is_enabled;

                    /** Variance of the observations. */
                    template <typename t_dictionary_type>
                    static double compute_variance(t_count_type norm, const t_dictionary_type& data, const t_mean_type& mean) noexcept
                    {
                        if (norm == 0) return std::numeric_limits<double>::signaling_NaN();

                        double variance_sum = t_mean_type(); // zero.
                        for (const auto& item : data)
                        {
                            variance_sum += ((item.first - mean) * (item.first - mean)) * item.second;
                        }
                        return variance_sum / norm;
                    }

                    /** Standard deviation of the observations. */
                    template <typename t_dictionary_type>
                    static double compute_standard_deviation(t_count_type norm, const t_dictionary_type& data, const t_mean_type& mean) noexcept
                    {
                        return std::sqrt(compute_variance(norm, data, mean));
                    }
                };

                /** When not supported, do nothing. */
                template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
                struct variance_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type, false>
                {
                    static constexpr bool is_enabled = false;
                };
            }
        }
    }
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_STATISTICS_BUILDER_HPP_INCLUDED
