
#ifndef ROPUFU_AFTERMATH_PROBABILITY_DICTIONARY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_PROBABILITY_DICTIONARY_HPP_INCLUDED

#include "../type_concepts.hpp"

#include <cstddef>
#include <map>
#include <type_traits>
#include <unordered_map>

namespace ropufu
{
    namespace aftermath
    {
        namespace probability
        {
            namespace detail
            {
                /** A counter dictionary type that switches between implementations depending on underlying type capabilities. */
                template <typename t_key_type, typename t_count_type>
                struct dictionary;

                /** Shorthand notation for counter dictionary type. */
                template <typename t_key_type, typename t_count_type>
                using dictionary_t = typename dictionary<t_key_type, t_count_type>::type;

                /** A counter dictionary type that switches between implementations depending on underlying type capabilities. */
                template <typename t_key_type, typename t_count_type>
                struct dictionary
                {
                    /** Checks if \c t_key_type implements operator "less". */
                    static constexpr bool is_ordered = type_impl::has_less<t_key_type>::value;

                    using key_type = t_key_type;
                    using count_type = t_count_type;
                    using type = typename type_impl::type_switch<is_ordered,
                        std::map<key_type, count_type>,
                        std::unordered_map<key_type, count_type>>::type;

                    /** Merges two counting dictionaries into \p target_data. */
                    template <typename t_dictionary_type>
                    static void merge(t_dictionary_type& target_data, const t_dictionary_type& source_data) noexcept
                    {
                        for (const auto& item : source_data)
                        {
                            auto it = target_data.find(item.first);
                            if (it != target_data.end())
                            {
                                auto height = (it->second) + item.second;
                                it->second = height;
                            }
                            else target_data.insert(item);
                        }
                    }

                    /** Merges two counting dictionaries into \p target_data with a twist. */
                    template <typename t_target_dictionary_type, typename t_source_dictionary_type, typename t_transformer_type>
                    static void merge(
                        t_target_dictionary_type& target_data,
                        const t_source_dictionary_type& source_data,
                        const t_transformer_type& transformer) noexcept
                    {
                        for (const auto& item : source_data)
                        {
                            auto value = transformer(item.first);
                            auto it = target_data.find(value);
                            if (it != target_data.end())
                            {
                                auto height = (it->second) + item.second;
                                it->second = height;
                            }
                            else target_data.insert(std::make_pair(value, item.second));
                        }
                    }

                    /** Finds an element in \p data with a given \p key, or returns \p default_value if not found. */
                    template <typename t_dictionary_type>
                    static count_type find_or_default(
                        const t_dictionary_type& data,
                        const typename t_dictionary_type::key_type& key,
                        const typename t_dictionary_type::mapped_type& default_value) noexcept
                    {
                        auto it = data.find(key);
                        return it != data.end() ? it->second : default_value;
                    }

                    /** Finds an element in \p data with a given \p key, or returns \p default_value if not found. */
                    template <typename t_dictionary_type>
                    static count_type find_or_default(
                        const t_dictionary_type& data,
                        const typename t_dictionary_type::key_type& key,
                        typename t_dictionary_type::mapped_type&& default_value) noexcept
                    {
                        auto it = data.find(key);
                        return it != data.end() ? it->second : default_value;
                    }

                    /** Copies the \p data into two arrays: keys and counts. */
                    template <typename t_dictionary_type>
                    static void copy(
                        const t_dictionary_type& data,
                        typename t_dictionary_type::key_type* p_keys,
                        typename t_dictionary_type::mapped_type* p_values) noexcept
                    {
                        std::size_t index = 0;
                        for (const auto& item : data)
                        {
                            p_keys[index] = item.first;
                            p_values[index] = item.second;
                            index++;
                        }
                    }

                    /** Copies the \p data into two arrays: keys and weights. */
                    template <typename t_dictionary_type>
                    static void copy(
                        const t_dictionary_type& data,
                        typename t_dictionary_type::key_type* p_keys,
                        double* p_values,
                        double norm) noexcept
                    {
                        std::size_t index = 0;
                        for (const auto& item : data)
                        {
                            p_keys[index] = item.first;
                            p_values[index] = item.second / norm;
                            index++;
                        }
                    }
                };
            }
        }
    }
}

#endif // ROPUFU_AFTERMATH_PROBABILITY_DICTIONARY_HPP_INCLUDED
