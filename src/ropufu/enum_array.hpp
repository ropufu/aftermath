
#ifndef ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "noexcept_json.hpp"

#include "concepts.hpp"
#include "enum_parser.hpp"
#include "key_value_pair.hpp"

#include <array>        // std::array
#include <cstddef>      // std::size_t, std::nullptr_t
#include <initializer_list> // std::initializer_list
#include <ostream>      // std::ostream
#include <stdexcept>    // std::out_of_range, std::logic_error
#include <string>       // std::string, std::to_string
#include <type_traits>  // std::underlying_type_t
#include <system_error> // std::error_code, std::errc
#include <utility>      // std::hash

namespace ropufu::aftermath
{
    namespace detail
    {
        /** @brief Auxiliary structure to be specialized for usage in \c enum_array. */
        template <ropufu::enumeration t_enum_type>
        struct enum_array_keys
        {
            using underlying_type = std::underlying_type_t<t_enum_type>;

            static constexpr underlying_type first_index = 0;
            static constexpr underlying_type past_the_last_index = 0;
        }; // struct enum_array_keys

        template <ropufu::enumeration t_enum_type, typename t_value_type>
        struct enum_array_core
        {
            using type = enum_array_core<t_enum_type, t_value_type>;
            using enum_type = t_enum_type;
            using value_type = t_value_type;

            using helper_type = detail::enum_array_keys<enum_type>;
            using underlying_type = std::underlying_type_t<enum_type>;

            static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
            static constexpr underlying_type first_index = helper_type::first_index;
            static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;

        protected:
            std::array<value_type, type::capacity> m_collection = {};

            const value_type& operator [](underlying_type k) const
            {
                k -= type::first_index;
                return this->m_collection[static_cast<std::size_t>(k)];
            } // operator [](...)

            value_type& operator [](underlying_type k)
            {
                k -= type::first_index;
                return this->m_collection[static_cast<std::size_t>(k)];
            } // operator [](...)

            bool within_bounds(underlying_type k) const noexcept
            {
                return (k >= type::first_index) && (k < type::past_the_last_index);
            } // within_bounds(...)

        public:
            enum_array_core() noexcept
            {
            } // enum_array_core(...)

            constexpr std::size_t size() const noexcept { return type::capacity; }
            constexpr std::size_t max_size() const noexcept { return type::capacity; }

            constexpr bool empty() const noexcept { return type::capacity == 0; }

            void fill(const value_type& value) noexcept { this->m_collection.fill(value); }

            bool within_bounds(enum_type index) const noexcept { return this->within_bounds(static_cast<underlying_type>(index)); }

            /** Checks if any of the elements of \c enum_array have non-default value. */
            bool zero() const noexcept
            {
                value_type z { };
                for (const value_type& x : this->m_collection) if (x != z) return false;
                return true;
            } // zero(...)
            
            const value_type& operator [](enum_type index) const { return this->operator [](static_cast<underlying_type>(index)); }
            value_type& operator [](enum_type index) { return this->operator [](static_cast<underlying_type>(index)); }

            /** @brief Locates the value at the position specified by \p index.
             *  @exception std::out_of_range \p index is outside the range of recognized values.
             */
            const value_type& at(enum_type index) const
            {
                underlying_type k = static_cast<underlying_type>(index);
                if (!this->within_bounds(k)) throw std::out_of_range("Index outside the range of recognized values.");
                return this->operator [](k);
            } // at(...)

            /** @brief Locates the value at the position specified by \p index.
             *  @exception std::out_of_range \p index is outside the range of recognized values.
             */
            value_type& at(enum_type index)
            {
                underlying_type k = static_cast<underlying_type>(index);
                if (!this->within_bounds(k)) throw std::out_of_range("Index outside the range of recognized values.");
                return this->operator [](k);
            } // at(...)

            bool operator !=(const type& other) const noexcept { return this->m_collection != other.m_collection; }
            bool operator ==(const type& other) const noexcept { return this->m_collection == other.m_collection; }
        }; // struct enum_array_core
    } // namespace detail
    
    /** @brief An iterator for \c enum_array usage in range-based for loops. */
    template <ropufu::enumeration t_enum_type, typename t_value_type, typename t_value_pointer_type>
    struct enum_array_iterator
    {
        using type = enum_array_iterator<t_enum_type, t_value_type, t_value_pointer_type>;
        using enum_type = t_enum_type;
        using value_type = t_value_type;
        using value_pointer_type = t_value_pointer_type;

        using helper_type = detail::enum_array_keys<enum_type>;
        using underlying_type = std::underlying_type_t<enum_type>;
        using iterator_result_type = detail::key_value_pair<enum_type, value_pointer_type>;

        static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
        static constexpr underlying_type first_index = helper_type::first_index;
        static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;

    private:
        value_pointer_type m_raw_data = nullptr;
        underlying_type m_position = type::first_index;

    public:
        enum_array_iterator(value_pointer_type collection_data_pointer, underlying_type position) noexcept
            : m_raw_data(collection_data_pointer), m_position(position)
        {
            static_assert(type::capacity != 0, "ropufu::aftermath::detail::enum_array_keys<...> has to be specialized.");
        } // enum_array_iterator(...)

        /** Inequality operator, used as termination condition. */
        bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }
        /** Equality operator. */
        bool operator ==(const type& other) const noexcept { return this->m_position == other.m_position; }

        /** Returns the current enum key/value pair. */
        iterator_result_type operator *() const noexcept
        {
            return iterator_result_type(
                static_cast<enum_type>(this->m_position),
                &this->m_raw_data[static_cast<std::size_t>(this->m_position - type::first_index)]
            );
        } // operator *(...)

        /** If not at the end, advances the position of the iterator by one. */
        type& operator ++() noexcept
        {
            // if (this->m_position == type::past_the_last_index) return *this; // Technically unnecessary.
            ++(this->m_position);
            return *this;
        } // operator ++(...)
    }; // struct enum_array_iterator
    
    /** @brief A specialized iterator for Boolean-valued \c enum_array usage in range-based for loops. */
    template <ropufu::enumeration t_enum_type, typename t_value_pointer_type>
    struct enum_array_iterator<t_enum_type, bool, t_value_pointer_type>
    {
        using type = enum_array_iterator<t_enum_type, bool, t_value_pointer_type>;
        using enum_type = t_enum_type;
        using value_type = t_enum_type;
        using value_pointer_type = const bool*;

        using helper_type = detail::enum_array_keys<enum_type>;
        using underlying_type = std::underlying_type_t<enum_type>;
        using iterator_result_type = enum_type; // Iterates over the marked enum values.

        static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
        static constexpr underlying_type first_index = helper_type::first_index;
        static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;

    private:
        value_pointer_type m_raw_data = nullptr;
        underlying_type m_position = type::first_index;

        /** Advances the pointer to the next set flag (if any). */
        void advance_to_true() noexcept
        {
            std::size_t array_index = static_cast<std::size_t>(this->m_position - type::first_index);
            while (array_index != type::capacity)
            {
                bool current = this->m_raw_data[array_index];
                if (current) break;
                ++array_index;
                ++this->m_position;
            } // while(...)
        } // advance_to_true(...)

    public:
        enum_array_iterator(value_pointer_type collection_data_pointer, underlying_type position) noexcept
            : m_raw_data(collection_data_pointer), m_position(position)
        {
            static_assert(type::capacity != 0, "ropufu::aftermath::detail::enum_array_keys<...> has to be specialized.");
            this->advance_to_true();
        } // enum_array_iterator(...)

        /** Inequality operator, used as termination condition. */
        bool operator !=(const type& other) const noexcept { return this->m_position != other.m_position; }
        /** Equality operator. */
        bool operator ==(const type& other) const noexcept { return this->m_position == other.m_position; }

        /** Dereferencing return current position. */
        iterator_result_type operator *() const noexcept { return static_cast<enum_type>(this->m_position); }

        /** Advances the pointer to the next set flag (if any). */
        type& operator ++() noexcept
        {
            if (this->m_position == type::past_the_last_index) return *this;
            ++(this->m_position);
            this->advance_to_true();
            return *this;
        } // operator ++(...)
    }; // struct enum_array_iterator<...>
    
    /** @brief An array indexed by \tparam t_enum_type. */
    template <ropufu::enumeration t_enum_type, typename t_value_type>
    struct enum_array : public detail::enum_array_core<t_enum_type, t_value_type>
    {
        using type = enum_array<t_enum_type, t_value_type>;
        using enum_type = t_enum_type;
        using value_type = t_value_type;

        using helper_type = detail::enum_array_keys<enum_type>;
        using underlying_type = std::underlying_type_t<enum_type>;
        using iterator_type = enum_array_iterator<enum_type, value_type, value_type*>;
        using const_iterator_type = enum_array_iterator<enum_type, value_type, const value_type*>;

        static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
        static constexpr underlying_type first_index = helper_type::first_index;
        static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;
        
        friend struct ropufu::noexcept_json_serializer<type>;

        enum_array() noexcept { }
        explicit enum_array(const value_type& value) noexcept { this->m_collection.fill(value); }

        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        iterator_type begin() noexcept { return iterator_type(this->m_collection.data(), type::first_index); }
        iterator_type end() noexcept { return iterator_type(this->m_collection.data(), type::past_the_last_index); }

        std::size_t get_hash() const noexcept
        {
            std::size_t result = 0;
            std::hash<value_type> value_hash = {};
            for (const value_type& x : this->m_collection)
            {
                result ^= value_hash(x);
                result <<= 1;
            } // for (...)
            return result;
        } // get_hash(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array

    /** @brief Masks enumerable keys of \tparam t_enum_type. */
    template <ropufu::enumeration t_enum_type>
    using flags_t = enum_array<t_enum_type, bool>;

    /** @brief Masks enumerable keys of \tparam t_enum_type. */
    template <ropufu::enumeration t_enum_type>
    struct enum_array<t_enum_type, bool> : public detail::enum_array_core<t_enum_type, bool>
    {
        using type = enum_array<t_enum_type, bool>;
        using enum_type = t_enum_type;
        using value_type = bool;

        using helper_type = detail::enum_array_keys<enum_type>;
        using underlying_type = std::underlying_type_t<enum_type>;
        using iterator_type = enum_array_iterator<enum_type, value_type, const value_type*>;
        using const_iterator_type = enum_array_iterator<enum_type, value_type, const value_type*>;

        static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
        static constexpr underlying_type first_index = helper_type::first_index;
        static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;

        friend struct ropufu::noexcept_json_serializer<type>;

        enum_array() noexcept { }

        /*implicit*/ enum_array(std::initializer_list<enum_type> flags) noexcept
        {
            for (enum_type flag : flags)
            {
                underlying_type k = static_cast<underlying_type>(flag);
                if (!this->within_bounds(k)) continue;
                this->operator [](k) = true;
            } // for (...)
        } // enum_array

        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        bool has(enum_type flag) const noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->within_bounds(k)) return false;
            return this->operator [](k);
        } // has(...)

        void set(enum_type flag) noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->within_bounds(k)) return;
            this->operator [](k) = true;
        } // set(...)

        void unset(enum_type flag) noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->within_bounds(k)) return;
            this->operator [](k) = false;
        } // unset(...)

        /** Flip all the flags. */
        void flip() noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] = !this->m_collection[i];
        } // flip(...)

        /** Elementwise "not". */
        type operator !() const noexcept
        {
            type result = *this;
            for (std::size_t i = 0; i < type::capacity; ++i) result.m_collection[i] = !result.m_collection[i];
            return result;
        } // operator !(...)

        /** Elementwise "or". */
        type& operator |=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] |= other.m_collection[i];
            return *this;
        } // operator |=(...)

        /** Elementwise "and". */
        type& operator &=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] &= other.m_collection[i];
            return *this;
        } // operator &=(...)

        /** Elementwise "exclusive or". */
        type& operator ^=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] ^= other.m_collection[i];
            return *this;
        } // operator ^=(...)

        friend type operator |(type left, const type& right) noexcept { left |= right; return left; }
        friend type operator &(type left, const type& right) noexcept { left &= right; return left; }
        friend type operator ^(type left, const type& right) noexcept { left ^= right; return left; }

        std::size_t get_hash() const noexcept
        {
            std::size_t result = 0;
            std::size_t one = 1;
            for (const bool& x : this->m_collection)
            {
                if (x) result ^= one;
                one <<= 1;
            } // for (...)
            return result;
        } // get_hash(...)

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array<...>

    /** @brief Lists enumerable keys of \tparam t_enum_type. */
    template <ropufu::enumeration t_enum_type>
    struct enum_array<t_enum_type, void>
    {
        using type = enum_array<t_enum_type, void>;
        using enum_type = t_enum_type;
        using value_type = t_enum_type;

        using helper_type = detail::enum_array_keys<enum_type>;
        using underlying_type = std::underlying_type_t<enum_type>;

        static constexpr std::size_t capacity = static_cast<std::size_t>(helper_type::past_the_last_index - helper_type::first_index);
        static constexpr underlying_type first_index = helper_type::first_index;
        static constexpr underlying_type past_the_last_index = helper_type::past_the_last_index;

        friend struct ropufu::noexcept_json_serializer<type>;

    private:
        std::array<value_type, type::capacity> m_collection = { };

        explicit enum_array(std::nullptr_t) noexcept
        {
            for (underlying_type i = type::first_index; i < type::past_the_last_index; ++i)
                this->m_collection[static_cast<std::size_t>(i - type::first_index)] = static_cast<enum_type>(i);
        } // enum_array(...)

    public:
        enum_array() noexcept
        {
            static type s_instance(nullptr); // Populate the list only once.
            this->m_collection = s_instance.m_collection;
        } // enum_array(...)

        constexpr std::size_t size() const noexcept { return type::capacity; }
        constexpr std::size_t max_size() const noexcept { return type::capacity; }

        constexpr bool empty() const noexcept { return type::capacity == 0; }
        
        const value_type& at(std::size_t index) const { return this->m_collection.at(index); }

        const value_type& operator [](std::size_t index) const { return this->m_collection[index]; }

        auto cbegin() const noexcept { return this->m_collection.cbegin(); }
        auto cend() const noexcept { return this->m_collection.cend(); }

        auto begin() const noexcept { return this->m_collection.cbegin(); }
        auto end() const noexcept { return this->m_collection.cend(); }

        bool operator !=(const type& other) const noexcept { return this->m_collection != other.m_collection; }
        bool operator ==(const type& other) const noexcept { return this->m_collection == other.m_collection; }

        constexpr std::size_t get_hash() const noexcept { return 0; }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array<...>

    /** Store as an object { ..., "<enum key>": value, ... }. */
    template <ropufu::enumeration t_enum_type, typename t_value_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, t_value_type>& x) noexcept
    {
        j = {};
        t_value_type z {};
        for (const auto& pair : x)
        {
            if (pair.value() == z) continue; // Skip default values.
            j[detail::enum_parser<t_enum_type>::to_string(pair.key())] = pair.value();
        } // for (...)
    } // to_json(...)

    /** Store as an array [ ..., "<enum key>", ... ]. */
    template <ropufu::enumeration t_enum_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, bool>& x) noexcept
    {
        std::vector<std::string> y {};
        y.reserve(enum_array<t_enum_type, bool>::capacity);
        for (t_enum_type value : x) y.push_back(detail::enum_parser<t_enum_type>::to_string(value));
        j = y;
    } // to_json(...)

    /** Store as an array [ ..., "<enum key>", ... ]. */
    template <ropufu::enumeration t_enum_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, void>& x) noexcept
    {
        std::vector<std::string> y {};
        y.reserve(enum_array<t_enum_type, void>::capacity);
        for (t_enum_type value : x) y.push_back(detail::enum_parser<t_enum_type>::to_string(value));
        j = y;
    } // to_json(...)

    template <ropufu::enumeration t_enum_type, typename t_value_type>
    void from_json(const nlohmann::json& j, enum_array<t_enum_type, t_value_type>& x)
    {
        if (!noexcept_json::try_get(j, x)) throw std::runtime_error("Parsing <interval> failed: " + j.dump());
    } // from_json(...)
} // namespace ropufu::aftermath

namespace ropufu
{
    template <ropufu::enumeration t_enum_type, typename t_value_type>
    struct noexcept_json_serializer<ropufu::aftermath::enum_array<t_enum_type, t_value_type>>
    {
        using enum_type = t_enum_type;
        using value_type = t_value_type;
        using result_type = ropufu::aftermath::enum_array<t_enum_type, t_value_type>;
        using underlying_type = std::underlying_type_t<t_enum_type>;

        /** Unpack JSON object { ..., "<enum key>": value, ... }. */
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            for (underlying_type k = result_type::first_index; k < result_type::past_the_last_index; ++k)
            {
                enum_type key = static_cast<enum_type>(k);
                value_type value {};

                std::string key_str = aftermath::detail::enum_parser<enum_type>::to_string(key);
                if (!noexcept_json::optional(j, key_str, value)) return false;
                x[k] = value;
            } // for (...)
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>

    template <ropufu::enumeration t_enum_type>
    struct noexcept_json_serializer<ropufu::aftermath::enum_array<t_enum_type, bool>>
    {
        using enum_type = t_enum_type;
        using value_type = bool;
        using result_type = ropufu::aftermath::enum_array<t_enum_type, bool>;
        using underlying_type = std::underlying_type_t<t_enum_type>;

        /** Unpack JSON array [ ..., "<enum key>", ... ]. */
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            std::vector<std::string> str_vector {};
            if (!noexcept_json::try_get(j, str_vector)) return false;

            for (const std::string& key_str : str_vector)
            {
                enum_type key {};
                if (!aftermath::detail::enum_parser<enum_type>::try_parse(key_str, key)) return false;
                x[key] = true;
            } // for (...)
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>

    template <ropufu::enumeration t_enum_type>
    struct noexcept_json_serializer<ropufu::aftermath::enum_array<t_enum_type, void>>
    {
        using enum_type = t_enum_type;
        using value_type = void;
        using result_type = ropufu::aftermath::enum_array<t_enum_type, void>;
        using underlying_type = std::underlying_type_t<t_enum_type>;

        /** Unpack JSON array [ ..., "<enum key>", ... ]. */
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            std::vector<std::string> str_vector = {};
            if (!noexcept_json::try_get(j, str_vector)) return false;

            /** @todo Check existing strings in \c str_vector against \c enum_array values. */
            if (str_vector.size() != result_type::capacity) return false;

            static result_type s_instance(nullptr); // Populate the list only once.
            x.m_collection = s_instance.m_collection;
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu

namespace std
{
    template <ropufu::enumeration t_enum_type, typename t_value_type>
    std::string to_string(const ropufu::aftermath::enum_array<t_enum_type, t_value_type>& value) noexcept
    {
        nlohmann::json j = value;
        return j.dump();
    } // to_string(...)

    template <ropufu::enumeration t_enum_type, typename t_value_type>
    struct hash<ropufu::aftermath::enum_array<t_enum_type, t_value_type>>
    {
        using argument_type = ropufu::aftermath::enum_array<t_enum_type, t_value_type>;
        using result_type = std::size_t;

        result_type operator ()(argument_type const& x) const noexcept
        {
            return x.get_hash();
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED
