
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED

#include "../concepts.hpp"
#include "../number_traits.hpp"

#include <array>     // std::array
#include <cmath>     // std::sqrt, std::round
#include <concepts>  // std::totally_ordered
#include <cstddef>   // std::size_t
#include <iostream>  // std::ostream, std::endl
#include <limits>    // std::numeric_limits
#include <map>       // std::map
#include <stdexcept> // std::logic_error
#include <string>    // std::string
#include <type_traits>   // std::void_t, std::conditional_t
#include <unordered_map> // std::unordered_map
#include <utility>       // std::declval

namespace ropufu::aftermath::probability
{
    namespace detail
    {
        template <typename, typename, typename = void> 
        struct product_result_type
        {
            using type = void;
            static constexpr bool value = false;
        }; // struct product_result_type

        template <typename t_left_type, typename t_right_type>
        struct product_result_type<t_left_type, t_right_type, std::void_t<decltype( std::declval<t_left_type>() * std::declval<t_right_type>() )>>
        {
            using type = decltype( std::declval<t_left_type>() * std::declval<t_right_type>() );
            static constexpr bool value = true;
        }; // struct product_result_type<...>

        template <typename t_left_type, typename t_right_type>
        using product_result_t = typename product_result_type<t_left_type, t_right_type>::type;
            
        template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        concept linear_space = ropufu::closed_under_division<t_mean_type> &&
            ropufu::zero_assignable<t_sum_type> && ropufu::zero_assignable<t_mean_type> && 
            std::convertible_to<t_count_type, t_mean_type> && std::convertible_to<t_sum_type, t_mean_type> &&
            requires(const t_key_type& x, const t_count_type& repeat, const t_sum_type& sum)
            {
                {sum + static_cast<t_sum_type>(x)} -> std::same_as<t_sum_type>;
                {sum + static_cast<t_sum_type>(repeat * x)} -> std::same_as<t_sum_type>;
            }; // concept linear_space
            
        template <typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        concept quadratic_space = linear_space<t_key_type, t_count_type, t_sum_type, t_mean_type> &&
            ropufu::closed_under_subtraction<t_mean_type> && ropufu::closed_under_multiplication<t_mean_type>;

        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_core
        {
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using dictionary_type = std::conditional_t<std::totally_ordered<t_key_type>,
                std::map<t_key_type, t_count_type>,
                std::unordered_map<t_key_type, t_count_type>>;

        protected:
            dictionary_type m_data = {};
            count_type m_count_observations = 0;
            count_type m_max_height = 0;
            key_type m_most_likely_value = {};

        public:
            void clear() noexcept
            {
                this->m_data.clear();
                this->m_count_observations = 0;
                this->m_max_height = 0;

                t_derived_type* that = static_cast<t_derived_type*>(this);
                that->on_cleared();
            } // clear(...)

            /** Observe \p repeat occurences of \p \key. */
            void observe(const key_type& key, count_type repeat = 1) noexcept
            {
                if (repeat == 0) return;
                count_type new_height = (this->m_data[key] += repeat);
                this->m_count_observations += repeat;
                if (this->m_max_height < new_height)
                {
                    this->m_max_height = new_height;
                    this->m_most_likely_value = key;
                } // if (...)

                t_derived_type* that = static_cast<t_derived_type*>(this);
                that->on_observed(key, repeat, new_height);
            } // observe(...)
            
            t_derived_type& operator <<(const key_type& key) noexcept
            {
                this->observe(key);
                t_derived_type* that = static_cast<t_derived_type*>(this);
                return *that;
            } // operator <<(...)

            /** Indicates if any observation has been made. */
            bool empty() const noexcept { return this->m_count_observations == 0; }

            /** Count the total number of observations. */
            count_type count() const noexcept { return this->m_count_observations; }

            /** Count the number of times \p key has been observed. */
            count_type count(const key_type& key) const noexcept
            {
                auto it = this->m_data.find(key);
                return (it != this->m_data.end()) ? (it->second) : 0;
            } // count(...)
            
            probability_type pmf(const key_type& key) const noexcept
            {
                auto it = this->m_data.find(key);
                return (it != this->m_data.end()) ? ((it->second) / static_cast<probability_type>(this->m_count_observations)) : 0;
            } // pmf(...)

            probability_type operator [](const key_type& key) const noexcept { return this->pmf(key); }

            /** The greatest observed empirical probability. */
            probability_type max_probability() const noexcept { return this->m_max_height / static_cast<probability_type>(this->m_count_observations); }

            /** The greatest observed count of some key. */
            count_type most_likely_count() const noexcept { return this->m_max_height; }

            /** The most observed value. */
            const key_type& most_likely_value() const noexcept { return this->m_most_likely_value; }

            /** The observation data. */
            const dictionary_type& data() const noexcept { return this->m_data; }
        }; // struct empirical_measure_core

        /** @brief Streaming module for \c empirical_measure. */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type, typename t_mean_type>
        struct empirical_measure_streaming_module
        {
            using type = empirical_measure_streaming_module<t_derived_type, t_key_type, t_count_type, t_probability_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;

            static constexpr bool is_enabled = false;

            friend std::ostream& operator <<(std::ostream& os, const derived_type& self)
            {
                return os << '{' << self.count() << " observations" << '}';
            } // operator <<(...)
        }; // struct empirical_measure_streaming_module

        /** @brief Streaming module for \c empirical_measure. */
        template <typename t_derived_type, ropufu::streamable t_key_type, typename t_count_type, typename t_probability_type, typename t_mean_type>
        struct empirical_measure_streaming_module<t_derived_type, t_key_type, t_count_type, t_probability_type, t_mean_type>
        {
            using type = empirical_measure_streaming_module<t_derived_type, t_key_type, t_count_type, t_probability_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using mean_type = t_mean_type;

            static constexpr bool is_enabled = true;

            friend std::ostream& operator <<(std::ostream& os, const derived_type& self)
            {
                constexpr std::size_t min_height = 1;
                constexpr std::size_t max_height = 25;
                constexpr std::size_t count_bins = 10;

                if (self.empty()) return os << "{no observations}";
                if (self.data().size() == 1) return os << '{' << self.data().cbegin()->first << ": all " << self.count() << " observations}";

                if constexpr (ropufu::integer<key_type>)
                {
                    probability_type scale = self.max_probability();
                    for (key_type key = self.min(); key <= self.max(); ++key)
                    {
                        probability_type p = self.pmf(key);
                        std::size_t height = min_height + static_cast<std::size_t>((p / scale) * (max_height - min_height));

                        os << std::string(height, '.') << std::string((1 + max_height) - height, ' ') <<
                            std::round(1000 * p) / 10 << '%' <<
                            '\t' << key << std::endl;
                    } // for (...)
                    return os;
                } // if constexpr (...)
                else if constexpr (std::totally_ordered<key_type> && ropufu::field<mean_type>)
                {
                    std::array<probability_type, count_bins> bars {};

                    mean_type from = self.min();
                    mean_type to = self.max();
                    mean_type step = (to - from) / static_cast<mean_type>(count_bins);
                    if (step == 0) os << '{' << from << " : 100%}" << std::endl;
                    else
                    {
                        for (const auto& item : self.data())
                        {
                            std::size_t bar_index = static_cast<std::size_t>((static_cast<mean_type>(item.first) - from) / step);
                            if (bar_index >= count_bins) bar_index = count_bins - 1;
                            probability_type p = self.pmf(item.first);
                            bars[bar_index] += p;
                        } // for (...)

                        probability_type scale = 0;
                        for (probability_type p : bars) if (p > scale) scale = p;

                        for (std::size_t i = 0; i < count_bins; ++i)
                        {
                            probability_type p = bars[i];
                            key_type label_a = static_cast<key_type>(from + i * step);
                            key_type label_b = static_cast<key_type>(from + (i + 1) * step);
                            std::size_t height = min_height + static_cast<std::size_t>((p / scale) * (max_height - min_height));

                            os << std::string(height, '.') << std::string((1 + max_height) - height, ' ') <<
                                std::round(1000 * p) / 10 << '%' <<
                                '\t' << label_a << "--" << label_b << std::endl;
                        } // for (...)
                    } // else (...)
                    return os;
                } // if constexpr (...)
                else
                {
                    probability_type norm = static_cast<probability_type>(self.count());
                    for (const auto& item : self.data())
                    {
                        probability_type p = static_cast<probability_type>(item.second) / norm;
                        os << '{' << item.first << " : " << (100 * p) << "%}" << std::endl;
                    } // for (...)
                    return os;
                } // if constexpr (...)
            } // operator <<(...)
        }; // struct empirical_measure_streaming_module

        /** @brief Ordering module for \c empirical_measure. */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_ordering_module
        {
            using type = empirical_measure_ordering_module<t_derived_type, t_key_type, t_count_type, t_probability_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using limits_type = std::numeric_limits<t_key_type>;

            static constexpr bool is_enabled = false;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_ordering_module

        /** @brief Ordering module for \c empirical_measure when \tparam t_key_type supports ordering. */
        template <typename t_derived_type, std::totally_ordered t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_ordering_module<t_derived_type, t_key_type, t_count_type, t_probability_type>
        {
            using type = empirical_measure_ordering_module<t_derived_type, t_key_type, t_count_type, t_probability_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using limits_type = std::numeric_limits<t_key_type>;

            static constexpr bool is_enabled = true;

        protected:
            key_type m_min = limits_type::max();
            key_type m_max = limits_type::lowest();

            /** Clears the statistics. */
            void module_clear() noexcept
            {
                this->m_min = limits_type::max();
                this->m_max = limits_type::lowest();
            } // module_clear(...)

            /** Observe another element. */
            void module_observe(const key_type& key, count_type /*repeat*/) noexcept
            {
                if (key < this->m_min) this->m_min = key;
                if (key > this->m_max) this->m_max = key;
            } // module_observe(...)

        public:
            /** Smallest observed key. */
            const key_type& min() const noexcept { return this->m_min; }
            /** Largest observed key. */
            const key_type& max() const noexcept { return this->m_max; }

            /** Compute empirical cumulative distribution function (c.d.f.). */
            probability_type cdf(const key_type& key) const noexcept
            {
                if (key < this->m_min) return 0;
                if (!(key < this->m_max)) return 1;

                const t_derived_type* that = static_cast<const t_derived_type*>(this);

                count_type cumulative_count = 0;
                for (const auto& item : that->m_data)
                {
                    if (key < item.first) break;
                    cumulative_count += item.second;
                } // for (...)
                return cumulative_count / static_cast<probability_type>(that->m_count_observations);
            } // cdf(...)

            /** Compute empirical percentile. */
            const t_key_type& percentile(probability_type probability) const
            {
                if (!aftermath::is_finite(probability)) throw std::logic_error("Probability must be a finite number.");
                if (probability < 0 || probability > 1) throw std::logic_error("Probability must be a finite number between 0 and 1.");
                if (probability == 0) return this->m_min;
                if (probability == 1) return this->m_max;

                const t_derived_type* that = static_cast<const t_derived_type*>(this);

                // Up-scale probability to [0, count].
                probability *= that->m_count_observations;
                count_type threshold = static_cast<count_type>(probability);
                if constexpr (std::numeric_limits<count_type>::is_integer) // For integer types we need a ceiling, not floor.
                {
                    if (threshold < probability) ++threshold;
                } // if constexpr (...)

                count_type cumulative_count = 0;
                for (const auto& item : that->m_data)
                {
                    cumulative_count += item.second;
                    if (cumulative_count >= threshold) return item.first;
                } // for (...)
                return this->m_max;
            } // percentile(...)
        }; // struct empirical_measure_ordering_module<...>

        /** @brief Linear module for \c empirical_measure. */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_linear_module
        {
            using type = empirical_measure_linear_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

            static constexpr bool is_enabled = false;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_linear_module
        
        /** @brief Linear module for \c empirical_measure when \tparam t_key_type supports linear operations (addition / subtraction / scaling). */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
            requires detail::linear_space<t_key_type, t_count_type, t_sum_type, t_mean_type>
        struct empirical_measure_linear_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>
        {
            using type = empirical_measure_linear_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

            static constexpr bool is_enabled = true;

        protected:
            sum_type m_sum = 0;

            void module_clear() noexcept { this->m_sum = 0; }
            void module_observe(const key_type& key, count_type repeat) noexcept
            {
                this->m_sum += static_cast<sum_type>(repeat * key);
            } // module_observe(...)

        public:
            /** Sum of the observations. */
            const sum_type& sum() const noexcept { return this->m_sum; }

            /** Mean of the observations. */
            mean_type mean() const noexcept 
            {
                const t_derived_type* that = static_cast<const t_derived_type*>(this);
                return static_cast<mean_type>(this->m_sum) / static_cast<mean_type>(that->m_count_observations);
            } // mean(...)
        }; // struct empirical_measure_linear_module<...>

        /** @brief Variance module for \c empirical_measure. */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_variance_module
        {
            using type = empirical_measure_variance_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

            static constexpr bool is_enabled = false;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_variance_module
        
        /** @brief Variance module for \c empirical_measure when \tparam t_key_type supports quadratic operations (multiplication). */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
            requires detail::quadratic_space<t_key_type, t_count_type, t_sum_type, t_mean_type>
        struct empirical_measure_variance_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>
        {
            using type = empirical_measure_variance_module<t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

            static constexpr bool is_enabled = true;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }

        public:
            mean_type compute_variance() const noexcept
            {
                const t_derived_type* that = static_cast<const t_derived_type*>(this);
                if (that->m_count_observations == 0) return std::numeric_limits<mean_type>::signaling_NaN();

                mean_type m = that->mean();
                mean_type variance_sum = 0;
                for (const auto& item : that->m_data)
                {
                    mean_type x = static_cast<mean_type>(item.first) - m;
                    x *= x;
                    variance_sum += item.second * x;
                } // for (...)
                return variance_sum / static_cast<mean_type>(that->m_count_observations);
            } // compute_variance(...)
            
            /** Standard deviation of the observations. */
            mean_type compute_standard_deviation() const noexcept { return std::sqrt(this->compute_variance); }
        }; // struct empirical_measure_variance_module<...>
    } // namespace detail

    /** @breif A structure to record observations and build up statistics. */
    template <ropufu::hashable t_key_type,
        typename t_count_type = std::size_t,
        typename t_probability_type = double,
        typename t_sum_type = detail::product_result_t<t_key_type, t_count_type>,
        typename t_mean_type = detail::product_result_t<t_key_type, t_probability_type>>
    struct empirical_measure
        : public detail::empirical_measure_core<
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_probability_type>,
        public detail::empirical_measure_streaming_module<
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_probability_type, t_mean_type>,
        public detail::empirical_measure_ordering_module<
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_probability_type>,
        public detail::empirical_measure_linear_module<
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_sum_type, t_mean_type>,
        public detail::empirical_measure_variance_module<
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_sum_type, t_mean_type>
    {
        using type = empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>;

        using key_type = t_key_type;
        using count_type = t_count_type;
        using probability_type = t_probability_type;
        using sum_type = t_sum_type;
        using mean_type = t_mean_type;

        using ordering_module = detail::empirical_measure_ordering_module<type, t_key_type, t_count_type, t_probability_type>;
        using linear_module = detail::empirical_measure_linear_module<type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
        using variance_module = detail::empirical_measure_variance_module<type, t_key_type, t_count_type, t_sum_type, t_mean_type>;

        template <ropufu::hashable, typename, typename, typename, typename>
        friend struct empirical_measure;

        template <typename, typename, typename, typename>
        friend struct detail::empirical_measure_core;

        template <typename, typename, typename, typename> friend struct detail::empirical_measure_ordering_module;
        template <typename, typename, typename, typename, typename> friend struct detail::empirical_measure_linear_module;
        template <typename, typename, typename, typename, typename> friend struct detail::empirical_measure_variance_module;

    private:
        void on_observed(const key_type& key, count_type repeat, count_type /*new_height*/) noexcept
        {
            this->ordering_module::module_observe(key, repeat);
            this->linear_module::module_observe(key, repeat);
            this->variance_module::module_observe(key, repeat);
        } // on_observed(...)

        void on_cleared() noexcept
        {
            this->ordering_module::module_clear();
            this->linear_module::module_clear();
            this->variance_module::module_clear();
        } // on_cleared(...)

    public:
        /** Construct a new empirical measure. */
        empirical_measure() noexcept { }
        
        /** Construct a new empirical measure from a dictionary. */
        template <typename t_dictionary_type>
        /*implicit*/ empirical_measure(const t_dictionary_type& data) noexcept
        {
            for (const auto& item : data) this->observe(item.first, item.second);
        } // empirical_measure(...)

        /** @brief Construct an empirical measure.
         *  @param ec Set to \c std::errc::invalid_argument if \p keys and \p values have different size.
         */
        template <typename t_key_container_type, typename t_value_container_type>
        empirical_measure(const t_key_container_type& keys, const t_value_container_type& values)
        {
            if (keys.size() != values.size()) throw std::logic_error("Keys and values have to be of the same size.");
            auto keys_it = keys.begin();
            auto values_it = values.begin();

            while (keys_it != keys.end() && values_it != values.end())
            {
                this->observe(*keys_it, *values_it);
                ++keys_it;
                ++values_it;
            } // while(...)
        } // empirical_measure(..)

        /** @brief Include observations from another empirical measure into this one. */
        void merge(const type& other) noexcept
        {
            for (const auto& item : other.m_data) this->observe(item.first, item.second);
        } // merge(...)
    }; // struct empirical_measure
} // namespace ropufu::aftermath::probability

#endif // ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
