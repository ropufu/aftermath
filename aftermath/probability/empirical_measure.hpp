
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED

#include "../not_an_error.hpp"
#include "../type_concepts.hpp"
#include "dictionary.hpp"
#include "statistics_builder.hpp"

#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <type_traits>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            namespace detail
            {
                /** @brief Ordering module for \c empirical_measure, intended as CRTP for static polymorhism.
                 *  @remark For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
                 */
                template <typename t_derived_type, typename t_key_type, typename t_probability_type>
                struct ordering_module
                {
                    using type = ordering_module<t_derived_type, t_key_type, t_probability_type>;
                    using derived_type = t_derived_type;

                protected:
                    detail::order_statistic<t_key_type, t_probability_type> m_order_statistic = { };

                public:
                    /** Smallest observed key. */
                    const t_key_type& min() const noexcept { return this->m_order_statistic.min(); }
                    /** Largest observed key. */
                    const t_key_type& max() const noexcept { return this->m_order_statistic.max(); }

                    /** Compute empirical cumulative distribution function (c.d.f.). */
                    t_probability_type cdf(const t_key_type& key) const noexcept
                    {
                        const derived_type* that = static_cast<const derived_type*>(this);
                        return this->m_order_statistic.cdf(key, that->m_count_observations, that->m_data);
                    }
                    
                    /** Compute empirical percentile. */
                    const t_key_type& percentile(t_probability_type probability) const noexcept
                    {
                        const derived_type* that = static_cast<const derived_type*>(this);
                        return this->m_order_statistic.percentile(probability, that->m_count_observations, that->m_data);
                    }
                };

                /** @brief Linear module for \c empirical_measure, intended as CRTP for static polymorhism.
                 *  @remark For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
                 */
                template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
                struct linear_module
                {
                    using type = linear_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
                    using derived_type = t_derived_type;

                protected:
                    detail::linear_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type> m_linear_statistic = { };

                public:
                    /** Sum of the observations. */
                    const t_sum_type& sum() const noexcept { return this->m_linear_statistic.sum(); }

                    /** Mean of the observations. */
                    t_mean_type mean() const noexcept 
                    {
                        const derived_type* that = static_cast<const derived_type*>(this);
                        return this->m_linear_statistic.mean(that->m_count_observations);
                    }
                };

                /** @brief Variance module for \c empirical_measure, intended as CRTP for static polymorhism.
                 *  @remark For more information on CRTP see https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
                 */
                template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
                struct variance_module
                {
                    using type = variance_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
                    using derived_type = t_derived_type;

                protected:
                    using variance_helper_type = detail::variance_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type>;

                public:
                    double compute_variance() const noexcept
                    {
                        const derived_type* that = static_cast<const derived_type*>(this);
                        return variance_helper_type::compute_variance(that->m_count_observations, that->m_data, that->mean());
                    }
                    
                    /** Standard deviation of the observations. */
                    double compute_standard_deviation() const noexcept
                    {
                        const derived_type* that = static_cast<const derived_type*>(this);
                        return variance_helper_type::compute_standard_deviation(that->m_count_observations, that->m_data, that->mean());
                    }
                };
            }

            /** Just to make the consequent templated code more readable. */
            #define ROPUFU_EMPIRICAL_MEASURE_TYPEDEF empirical_measure<t_key_type, t_count_type, t_sum_type, t_mean_type, t_probability_type, t_is_ordered, t_is_linear_space, t_has_variance>

            /** @breif A structure to record observations and build statistics.
             *  @remark The general struct implements the basic functionality meant to be shared amond specializations.
             */
            template <typename t_key_type,
                typename t_count_type = std::size_t,
                typename t_sum_type = typename type_impl::get_product_result<t_key_type, t_count_type>::type,
                //typename t_sum_type = typename type_impl::get_sum_result<t_key_type, t_count_type>::type,
                typename t_mean_type = typename type_impl::get_divide_result<t_sum_type, double>::type,
                typename t_probability_type = double,
                bool t_is_ordered = detail::order_statistic<t_key_type, t_probability_type>::is_enabled,
                bool t_is_linear_space = detail::linear_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type>::is_enabled,
                bool t_has_variance = detail::variance_statistic<t_key_type, t_count_type, t_sum_type, t_mean_type>::is_enabled>
            struct empirical_measure
                : public std::enable_if_t<t_is_ordered, detail::ordering_module<ROPUFU_EMPIRICAL_MEASURE_TYPEDEF, t_key_type, t_probability_type>>,
                public std::enable_if_t<t_is_linear_space, detail::linear_module<ROPUFU_EMPIRICAL_MEASURE_TYPEDEF, t_key_type, t_count_type, t_sum_type, t_mean_type>>,
                public std::enable_if_t<t_has_variance, detail::variance_module<ROPUFU_EMPIRICAL_MEASURE_TYPEDEF, t_key_type, t_count_type, t_sum_type, t_mean_type>>
            {
                static constexpr bool is_ordered = t_is_ordered; // Indicates if the underlying key type supports ordering.
                static constexpr bool is_linear_space = t_is_linear_space; // Indicates if the underlying key type supports linear operations (addition / subtraction / scaling).
                static constexpr bool has_variance = t_has_variance; // Indicates if the underlying key type supports quadratic operations (multiplication).
                
                using type = ROPUFU_EMPIRICAL_MEASURE_TYPEDEF;

                using key_type = t_key_type;
                using count_type = t_count_type;
                using sum_type = t_sum_type;
                using mean_type = t_mean_type;
                using probability_type = t_probability_type;
                
                using dictionary_type = typename detail::dictionary<key_type, count_type>::type;
                using variance_helper_type = detail::variance_statistic<key_type, count_type, sum_type, mean_type>;

                template <typename, typename, typename, typename, typename, bool, bool, bool>
                friend struct empirical_measure;

                template <typename, typename, typename>
                friend struct detail::ordering_module;

                template <typename, typename, typename, typename, typename>
                friend struct detail::linear_module;

                template <typename, typename, typename, typename, typename>
                friend struct detail::variance_module;

                static void validate() noexcept
                {
                    // key_type has to implement std::hash and equality.
                    static_assert(type_impl::has_equality<key_type>::value && type_impl::has_hash<key_type>::value,
                        "<t_key_type> has to implement binary '==' and std::hash.");
                }

            private:
                dictionary_type m_data = { };

                count_type m_count_observations = { };
                count_type m_max_height = { };
                key_type m_most_likely_value = { };

            public:
                /** Construct a new empirical measure. */
                empirical_measure() noexcept { type::validate(); }
                
                /** Construct a new empirical measure from a dictionary. */
                template <typename t_dictionary_type>
                empirical_measure(const t_dictionary_type& data) noexcept
                {
                    type::validate();
                    for (const auto& item : data) this->observe(transformer(item.first), item.second);
                }

                /** @brief Construct an empirical measure.
                 *  @remark \tparam t_key_container_type has to implement operator \c [std::size_t] -> \c key_type.
                 *  @remark \tparam t_value_container_type has to implement operator \c [std::size_t] -> \c count_type.
                 */
                template <typename t_key_container_type, typename t_value_container_type>
                empirical_measure(const t_key_container_type& keys, const t_value_container_type& values) noexcept
                {
                    type::validate();
                    if (keys.size() != values.size())
                    {
                        quiet_error::instance().push(
                            not_an_error::length_error,
                            severity_level::major,
                            "Observations size mismatch.", __FUNCTION__, __LINE__);
                        return;
                    }
                    for (std::size_t i = 0; i < keys.size(); i++) this->observe(keys[i], values[i]);
                }

                /** @brief Construct an empirical measure from an existing one.
                 *  @remark \tparam t_transformer_type has to implement operator (\tparam o_key_type) -> \tparam t_key_type.
                 */
                template <typename o_key_type, typename o_sum_type, typename o_mean_type, typename o_probability_type, bool o_is_ordered, bool o_is_linear_space, bool o_has_variance, typename t_transformer_type>
                empirical_measure(
                    const empirical_measure<o_key_type, count_type, o_sum_type, o_mean_type, o_probability_type, o_is_ordered, o_is_linear_space, o_has_variance>& other,
                    const t_transformer_type& transformer) noexcept
                {
                    type::validate();
                    for (const auto& item : other.m_data) this->observe(transformer(item.first), item.second);
                }

                /** Erase past observations. */
                void clear() noexcept
                {
                    this->m_data.clear();
                    this->m_count_observations = count_type();
                    this->m_max_height = count_type();

                    this->m_order_statistic.clear();
                    this->m_linear_statistic.clear();
                    //this->m_variance_statistic.clear();
                }

                /** @brief Include observations from another empirical measure into this one.
                 *  @remark \tparam t_transformer_type has to implement operator (\tparam o_key_type) -> \tparam t_key_type.
                 */
                template <typename o_key_type, typename o_sum_type, typename o_mean_type, typename o_probability_type, bool o_is_ordered, bool o_is_linear_space, bool o_has_variance, typename t_transformer_type>
                void merge(
                    const empirical_measure<o_key_type, count_type, o_sum_type, o_mean_type, o_probability_type, o_is_ordered, o_is_linear_space, o_has_variance>& other,
                    const t_transformer_type& transformer) noexcept
                {
                    detail::dictionary<key_type, count_type>::merge(this->m_data, other.m_data, transformer);
                    this->rebuild_statistic();
                }

                /** Include observations from another empirical measure into this one. */
                template <typename o_sum_type, typename o_mean_type, typename o_probability_type, bool o_is_ordered, bool o_is_linear_space, bool o_has_variance>
                void merge(const empirical_measure<key_type, count_type, o_sum_type, o_mean_type, o_probability_type, o_is_ordered, o_is_linear_space, o_has_variance>& other) noexcept
                {
                    detail::dictionary<key_type, count_type>::merge(this->m_data, other.m_data);
                    this->rebuild_statistic();
                }

                /** Count the number of times \p key has been observed. */
                count_type count_occurences(const key_type& key) const noexcept
                {
                    return detail::dictionary<key_type, count_type>::find_or_default(this->m_data, key, count_type());
                }

                /** Calculate the empirical probability of observing \p key. */
                probability_type operator[](const key_type& key) const noexcept
                {
                    return detail::dictionary<key_type, count_type>::find_or_default(this->m_data, key, count_type()) / static_cast<probability_type>(this->m_count_observations);
                }

                /** Copies the observation data into two arrays: keys and counts. */
                void copy_to(key_type* p_keys, count_type* p_counts) const noexcept
                {
                    detail::dictionary<key_type, count_type>::copy(this->m_data, p_keys, p_counts);
                }

                /** Copies the observation data into two arrays: keys and probabilities. */
                void copy_to_normalized(key_type* p_keys, probability_type* p_probabilities) const noexcept
                {
                    detail::dictionary<key_type, count_type>::copy(this->m_data, p_keys, p_probabilities, static_cast<probability_type>(this->m_count_observations));
                }

                /** Observe \p repeat occurences of \p \key. */
                void observe(const key_type& key, count_type repeat = 1) noexcept
                {
                    if (repeat == 0) return;
                    count_type new_height = repeat;

                    auto it = this->m_data.find(key);
                    bool is_contained = it != this->m_data.end();
                    if (is_contained) // Already contains key.
                    {
                        new_height += (it->second); // new_height = current_height + repeat.
                        it->second = new_height;
                    }

                    bool is_new_min = false;
                    bool is_new_max = false;
                    this->update_statistic(key, repeat, new_height, is_new_min, is_new_max);

                    if (is_contained) return; // No need to insert a new key.
                    
                    if (is_new_min)
                        this->m_data.insert(this->m_data.begin(), std::pair<t_key_type, count_type>(key, new_height));
                    else if (is_new_max)
                        this->m_data.insert(this->m_data.end(), std::pair<t_key_type, count_type>(key, new_height));
                    else 
                        this->m_data.insert(std::pair<t_key_type, count_type>(key, new_height));
                }

                /** Count the total number of observations. */
                count_type count_observations() const noexcept
                {
                    return this->m_count_observations;
                }

                /** Indicates if any observation has been made. */
                bool empty() const noexcept
                {
                    return this->m_count_observations == 0;
                }

                /** The greatest observed empirical probability. */
                probability_type max_probability() const noexcept
                {
                    return this->m_max_height / static_cast<probability_type>(this->m_count_observations);
                }

                /** The greatest observed count of some key. */
                count_type most_likely_count() const noexcept
                {
                    return this->m_max_height;
                }

                /** The most observed value. */
                const key_type& most_likely_value() const noexcept
                {
                    return this->m_most_likely_value;
                }

                /** The observation data. */
                const dictionary_type& data() const noexcept
                {
                    return this->m_data;
                }

                friend std::ostream& operator <<(std::ostream& os, const type& that) noexcept
                {
                    //if (that.m_count_observations == 0) return os << "{}";
                    that.print_to(os);
                    return os;
                }

            private:
                /** Updates the statistics based on the newest observation. */
                void update_statistic(const key_type& key, count_type repeat, count_type new_height, bool& is_new_min, bool& is_new_max) noexcept
                {
                    this->m_order_statistic.observe(key, std::ref(is_new_min), std::ref(is_new_max));
                    this->m_linear_statistic.observe(key, repeat);
                    //this->m_variance_statistic.observe(key, repeat);

                    if (this->m_max_height < new_height)
                    {
                        this->m_max_height = new_height;
                        this->m_most_likely_value = key;
                    }
                    this->m_count_observations += repeat;
                }

                /** Re-calculates statistics based on the stored data. */
                void rebuild_statistic() noexcept
                {
                    this->m_order_statistic.clear();
                    this->m_linear_statistic.clear();
                    //this->m_variance_statistic.clear();

                    bool is_new_min = false;
                    bool is_new_max = false;
                    this->m_count_observations = count_type();
                    this->m_max_height = count_type();
                    for (const auto& item : this->m_data) this->update_statistic(item.first, item.second, item.second, is_new_min, is_new_max);
                }

                /** @todo Figure out what this version of print does. */
                template <typename t_stream_type>
                typename std::enable_if<std::is_integral<key_type>::value && type_impl::has_left_shift<t_stream_type, key_type>::value>::type
                    print_to(t_stream_type& os, std::size_t min_height = 0, std::size_t max_height = 20) const noexcept
                {
                    count_type count = this->m_count_observations;
                    if (count == 0) os << "{}";
                    else
                    {
                        probability_type scale = this->max_probability();
                        for (key_type key = this->m_order_statistic.min(); key <= this->m_order_statistic.max(); ++key)
                        {
                            probability_type p = this->operator[](key);
                            std::size_t height = min_height + static_cast<std::size_t>((p / scale) * (max_height - min_height));
                            os << key << "\t" << std::string(height, '.') << std::string((1 + max_height) - height, ' ') << (100 * p) << '%' << std::endl;
                        }
                    }
                }

                /** @todo Figure out what this version of print does. */
                template <typename t_stream_type>
                typename std::enable_if<!std::is_integral<key_type>::value && type_impl::has_left_shift<t_stream_type, key_type>::value>::type
                    print_to(t_stream_type& os) const noexcept
                {
                    count_type count = this->m_count_observations;
                    if (count == 0) os << "{}";
                    else
                    {
                        probability_type norm = static_cast<probability_type>(count);
                        for (const auto& item : this->m_data)
                        {
                            probability_type p = item.second / norm;
                            os << "{" << item.first << " : " << (100 * p) << "%}" << std::endl;
                        }
                    }
                }

                /** @todo Figure out what this version of print does. */
                template <typename t_stream_type>
                typename std::enable_if<!type_impl::has_left_shift<t_stream_type, key_type>::value>::type
                    print_to(t_stream_type& os) const noexcept
                {
                    count_type count = this->m_count_observations;
                    if (count == 0) os << "{}";
                    else os << "{...}";
                }
            };
        }
    }
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
