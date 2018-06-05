
#ifndef ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED

#include "../on_error.hpp"
#include "../type_traits.hpp"

#include <cmath>     // std::sqrt
#include <cstddef>   // std::size_t
#include <iostream>  // std::ostream, std::endl
#include <limits>    // std::numeric_limits, std::numeric_limits::is_integer
#include <map>       // std::map
#include <string>    // std::string
#include <system_error>  // std::error_code, std::errc
#include <type_traits>   // ...
#include <unordered_map> // std::unordered_map

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

        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_core
        {
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using dictionary_type = std::conditional_t<type_traits::has_less_binary_v<t_key_type, t_key_type>,
                std::map<t_key_type, t_count_type>,
                std::unordered_map<t_key_type, t_count_type>>;

        protected:
            dictionary_type m_data = {};
            count_type m_count_observations = 0;
            count_type m_max_height = 0;
            key_type m_most_likely_value = { };

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
                ++this->m_count_observations;
                if (this->m_max_height < new_height)
                {
                    this->m_max_height = new_height;
                    this->m_most_likely_value = key;
                } // if (...)

                t_derived_type* that = static_cast<t_derived_type*>(this);
                that->on_observed(key, repeat, new_height);
            } // observe(...)

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

        /** @brief Ordering module for \c empirical_measure. */
        template <bool t_is_enabled, typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_ordering_module
        {
            using type = empirical_measure_ordering_module<t_is_enabled, t_derived_type, t_key_type, t_count_type, t_probability_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using limits_type = std::numeric_limits<t_key_type>;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_ordering_module

        /** @brief Ordering module for \c empirical_measure when \tparam t_key_type supports ordering. */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_probability_type>
        struct empirical_measure_ordering_module<true, t_derived_type, t_key_type, t_count_type, t_probability_type>
        {
            using type = empirical_measure_ordering_module<true, t_derived_type, t_key_type, t_count_type, t_probability_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using probability_type = t_probability_type;
            using limits_type = std::numeric_limits<t_key_type>;

        protected:
            key_type m_min = limits_type::max();
            key_type m_max = limits_type::min();

            /** Clears the statistics. */
            void module_clear() noexcept
            {
                this->m_min = limits_type::max();
                this->m_max = limits_type::min();
            } // module_clear(...)

            /** Observe another element. */
            void module_observe(const key_type& key, count_type /*repeat*/) noexcept
            {
                if (key < this->m_min) this->m_min = key;
                else if (this->m_max < key) this->m_max = key;
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
                }
                return cumulative_count / static_cast<probability_type>(that->m_count_observations);
            } // cdf(...)

            /** Compute empirical percentile. */
            const t_key_type& percentile(probability_type probability) const noexcept
            {
                if (probability <= 0) return this->m_min;
                if (probability >= 1) return this->m_max;

                const t_derived_type* that = static_cast<const t_derived_type*>(this);

                // Up-scale probability to [0, count].
                probability *= that->m_count_observations;
                count_type threshold = static_cast<count_type>(probability);
                if constexpr (std::numeric_limits<count_type>::is_integer) // For integer types we need a ceiling, not floor.
                {
                    if (threshold < probability) ++threshold;
                } // if constexpr

                count_type cumulative_count = 0;
                for (const auto& item : that->m_data)
                {
                    cumulative_count += item.second;
                    if (cumulative_count >= threshold) return item.first;
                }
                return this->m_max;
            } // percentile(...)
        }; // struct empirical_measure_ordering_module<...>

        /** @brief Linear module for \c empirical_measure. */
        template <bool t_is_enabled, typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_linear_module
        {
            using type = empirical_measure_linear_module<t_is_enabled, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_linear_module
        
        /** @brief Linear module for \c empirical_measure when \tparam t_key_type supports linear operations (addition / subtraction / scaling). */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_linear_module<true, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>
        {
            using type = empirical_measure_linear_module<true, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

        protected:
            sum_type m_sum = 0;

            void module_clear() noexcept { this->m_sum = 0; }
            void module_observe(const key_type& key, count_type repeat) noexcept
            {
                this->m_sum += static_cast<sum_type>(key * repeat);
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
        template <bool t_is_enabled, typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_variance_module
        {
            using type = empirical_measure_variance_module<t_is_enabled, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

        protected:
            void module_clear() noexcept { }
            void module_observe(const key_type& /*key*/, count_type /*repeat*/) noexcept { }
        }; // struct empirical_measure_variance_module
        
        /** @brief Variance module for \c empirical_measure when \tparam t_key_type supports quadratic operations (multiplication). */
        template <typename t_derived_type, typename t_key_type, typename t_count_type, typename t_sum_type, typename t_mean_type>
        struct empirical_measure_variance_module<true, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>
        {
            using type = empirical_measure_variance_module<true, t_derived_type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
            using derived_type = t_derived_type;
            using key_type = t_key_type;
            using count_type = t_count_type;
            using sum_type = t_sum_type;
            using mean_type = t_mean_type;

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
                    variance_sum += ((item.first - m) * (item.first - m)) * item.second;
                }
                return variance_sum / static_cast<mean_type>(that->m_count_observations);
            } // compute_variance(...)
            
            /** Standard deviation of the observations. */
            mean_type compute_standard_deviation() const noexcept { return std::sqrt(this->compute_variance); }
        }; // struct empirical_measure_variance_module<...>
    } // namespace detail

    /** @breif A structure to record observations and build statistics.
     *  @remark The general struct implements the basic functionality meant to be shared amond specializations.
     */
    template <typename t_key_type,
        typename t_count_type = std::size_t,
        typename t_probability_type = double,
        typename t_sum_type = detail::product_result_t<t_key_type, t_count_type>,
        typename t_mean_type = detail::product_result_t<t_key_type, t_probability_type>>
    struct empirical_measure
        : public detail::empirical_measure_core<empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>, t_key_type, t_count_type, t_probability_type>,
        public detail::empirical_measure_ordering_module<
            type_traits::has_less_binary_v<t_key_type, t_key_type>,
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_probability_type>,
        public detail::empirical_measure_linear_module<
            type_traits::has_multiply_binary_v<t_key_type, t_count_type>,
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_sum_type, t_mean_type>,
        public detail::empirical_measure_variance_module<
            type_traits::has_multiply_binary_v<t_key_type, t_key_type> && type_traits::has_multiply_binary_v<t_key_type, t_count_type>,
            empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>,
            t_key_type, t_count_type, t_sum_type, t_mean_type>
    {
        using type = empirical_measure<t_key_type, t_count_type, t_probability_type, t_sum_type, t_mean_type>;

        using key_type = t_key_type;
        using count_type = t_count_type;
        using probability_type = t_probability_type;
        using sum_type = t_sum_type;
        using mean_type = t_mean_type;

        using ordering_module = detail::empirical_measure_ordering_module<type_traits::has_less_binary_v<t_key_type, t_key_type>, type, t_key_type, t_count_type, t_probability_type>;
        using linear_module = detail::empirical_measure_linear_module<type_traits::has_multiply_binary_v<t_key_type, t_count_type>, type, t_key_type, t_count_type, t_sum_type, t_mean_type>;
        using variance_module = detail::empirical_measure_variance_module<type_traits::has_multiply_binary_v<t_key_type, t_key_type> && type_traits::has_multiply_binary_v<t_key_type, t_count_type>, type, t_key_type, t_count_type, t_sum_type, t_mean_type>;

        template <typename, typename, typename, typename, typename>
        friend struct empirical_measure;

        template <typename, typename, typename, typename>
        friend struct detail::empirical_measure_core;

        template <bool, typename, typename, typename, typename> friend struct detail::empirical_measure_ordering_module;
        template <bool, typename, typename, typename, typename, typename> friend struct detail::empirical_measure_linear_module;
        template <bool, typename, typename, typename, typename, typename> friend struct detail::empirical_measure_variance_module;

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
        empirical_measure(const t_key_container_type& keys, const t_value_container_type& values, std::error_code& ec) noexcept
        {
            if (keys.size() != values.size()) { aftermath::detail::on_error(ec, std::errc::invalid_argument, "Observations size mismatch."); return; }
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
            for (const auto& item : other.m_data) this->m_data[item.first] += item.second;
        } // merge(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            if (self.m_count_observations == 0) return os << "{ }";

            if constexpr (type_traits::has_left_shift_binary_v<decltype(os), key_type>)
            {
                if constexpr (type_traits::is_one_by_one_iterable_v<t_key_type>)
                {
                    constexpr std::size_t min_height = 5;
                    constexpr std::size_t max_height = 5;

                    probability_type scale = self.max_probability();
                    for (key_type key = self.min(); key <= self.max(); ++key)
                    {
                        probability_type p = self.pmf(key);
                        std::size_t height = min_height + static_cast<std::size_t>((p / scale) * (max_height - min_height));
                        os << key << "\t" << std::string(height, '.') << std::string((1 + max_height) - height, ' ') << (100 * p) << '%' << std::endl;
                    } // for (...)
                    return os;
                } // if constexpr
                else
                {
                    probability_type norm = static_cast<probability_type>(self.m_count_observations);
                    for (const auto& item : self.m_data)
                    {
                        probability_type p = static_cast<probability_type>(item.second) / norm;
                        os << "{" << item.first << " : " << (100 * p) << "%}" << std::endl;
                    } // for (...)
                    return os;
                } // if constexpr
            } // if constexpr

            return os << "{...}";
        } // operator <<(...)
    }; // struct empirical_measure
} // namespace ropufu::aftermath::probability

#endif // ROPUFU_AFTERMATH_PROBABILITY_EMPIRICAL_MEASURE_HPP_INCLUDED
