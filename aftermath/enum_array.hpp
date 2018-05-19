
#ifndef ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "quiet_json.hpp"
#include "not_an_error.hpp"

#include "enum_parser.hpp"
#include "key_value_pair.hpp"

#include <array>   // std::array
#include <cstddef> // std::size_t, std::nullptr_t
#include <cstring> // std::memcmp
#include <initializer_list> // std::initializer_list
#include <ostream> // std::ostream
#include <string>  // std::string, std::to_string
#include <type_traits> // std::underlying_type_t

namespace ropufu::aftermath
{
    namespace detail
    {
        /** @brief Auxiliary structure to be specialized for usage in \c enum_array. */
        template <typename t_enum_type>
        struct enum_array_keys
        {
            using underlying_type = std::underlying_type_t<t_enum_type>;

            static constexpr underlying_type first_index = 0;
            static constexpr underlying_type past_the_last_index = 0;
        }; // struct enum_array_keys

        template <typename t_enum_type, typename t_value_type>
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
            std::array<value_type, type::capacity + 1> m_collection = { }; // Make sure that dereferencing \c end() will not result in memory violation.

            const value_type& unchecked_at(enum_type index) const noexcept { return this->unchecked_at(static_cast<underlying_type>(index)); }
            const value_type& unchecked_at(underlying_type k) const noexcept
            {
                k -= type::first_index;
                return this->m_collection[static_cast<std::size_t>(k)];
            } // unchecked_at(...)

            value_type& unchecked_at(enum_type index) noexcept { return this->unchecked_at(static_cast<underlying_type>(index)); }
            value_type& unchecked_at(underlying_type k) noexcept
            {
                k -= type::first_index;
                return this->m_collection[static_cast<std::size_t>(k)];
            } // unchecked_at(...)

            bool is_within_range(enum_type index) const noexcept { return this->is_within_range(static_cast<underlying_type>(index)); }
            bool is_within_range(underlying_type k) const noexcept
            {
                return (k >= type::first_index) && (k < type::past_the_last_index);
            } // is_within_range(...)

        public:
            // Checks if the value is invalid.
            bool invalid(const value_type& value) const noexcept { return &value == &this->m_collection.back(); }

            constexpr std::size_t size() const noexcept { return type::capacity; }
            constexpr std::size_t max_size() const noexcept { return type::capacity; }

            constexpr bool empty() const noexcept { return type::capacity == 0; }

            void fill(const value_type& value) noexcept { this->m_collection.fill(value); }

            /** Checks if any of the elements of \c enum_array have non-default value. */
            bool zero() const noexcept
            {
                std::size_t index = 0;
                value_type z { };
                for (const value_type& x : this->m_collection)
                {
                    if (x != z) return false;
                    if (++index == type::capacity) return true; // Skip the "invalid" element.
                }
                return true;
            } // zero(...)

            /** Locates the value at the position specified by \p index.
             *  If \p index is outside the range of recognized values, returns "invalid" element,
             *  that can be checked by calling \c invalid(...) member function.
             */
            const value_type& at(enum_type index) const noexcept
            {
                underlying_type k = static_cast<underlying_type>(index);
                if (!this->is_within_range(k)) return this->m_collection.back(); // "Invalid" element.
                return this->unchecked_at(k);
            } // at(...)

            /** Locates the value at the position specified by \p index.
             *  If \p index is outside the range of recognized values, returns "invalid" element,
             *  that can be checked by calling \c invalid(...) member function.
             */
            value_type& at(enum_type index) noexcept
            {
                underlying_type k = static_cast<underlying_type>(index);
                if (!this->is_within_range(k)) return this->m_collection.back(); // "Invalid" element.
                return this->unchecked_at(k);
            } // at(...)
            
            const value_type& operator [](enum_type index) const { return this->unchecked_at(index); }
            value_type& operator [](enum_type index) { return this->unchecked_at(index); }

            bool operator !=(const type& other) const noexcept { return std::memcmp(this->m_collection.data(), other.m_collection.data(), type::capacity * sizeof(value_type)) != 0; }
            bool operator ==(const type& other) const noexcept { return std::memcmp(this->m_collection.data(), other.m_collection.data(), type::capacity * sizeof(value_type)) == 0; }
        }; // struct enum_array_core
    } // namespace detail
    
    /** @brief An iterator for \c enum_array usage in range-based for loops. */
    template <typename t_enum_type, typename t_value_type, typename t_value_pointer_type>
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
            if constexpr (type::capacity == 0)
            {
                static_assert(false, "ropufu::aftermath::detail::enum_array_keys<...> has to be specialized.");
            }
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
            if (this->m_position == type::past_the_last_index) return *this;
            ++(this->m_position);
            return *this;
        } // operator ++(...)
    }; // struct enum_array_iterator
    
    /** @brief A specialized iterator for Boolean-valued \c enum_array usage in range-based for loops. */
    template <typename t_enum_type, typename t_value_pointer_type>
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
            if constexpr (type::capacity == 0)
            {
                static_assert(false, "ropufu::aftermath::detail::enum_array_keys<...> has to be specialized.");
            }
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
    template <typename t_enum_type, typename t_value_type>
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

        enum_array() noexcept { }
        explicit enum_array(const value_type& value) noexcept { this->m_collection.fill(value); }

        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        iterator_type begin() noexcept { return iterator_type(this->m_collection.data(), type::first_index); }
        iterator_type end() noexcept { return iterator_type(this->m_collection.data(), type::past_the_last_index); }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array

    template <typename t_enum_type>
    using flags_t = enum_array<t_enum_type, bool>;

    /** @brief Masks enumerable keys of \tparam t_enum_type. */
    template <typename t_enum_type>
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

        enum_array() noexcept { }
        /*implicit*/ enum_array(std::initializer_list<enum_type> flags) noexcept
        {
            for (enum_type flag : flags)
            {
                underlying_type k = static_cast<underlying_type>(flag);
                if (!this->is_within_range(k)) continue;
                this->unchecked_at(k) = true;
            } // for (...)
        } // enum_array

        const_iterator_type cbegin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type cend() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        const_iterator_type begin() const noexcept { return const_iterator_type(this->m_collection.data(), type::first_index); }
        const_iterator_type end() const noexcept { return const_iterator_type(this->m_collection.data(), type::past_the_last_index); }

        bool has(enum_type flag) const noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->is_within_range(k)) return false;
            return this->unchecked_at(k);
        } // has(...)

        void set(enum_type flag) noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->is_within_range(k)) return;
            this->unchecked_at(k) = true;
        } // set(...)

        void unset(enum_type flag) noexcept
        {
            underlying_type k = static_cast<underlying_type>(flag);
            if (!this->is_within_range(k)) return;
            this->unchecked_at(k) = false;
        } // unset(...)

        /** Elementwise "or". */
        type& operator |=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] = (this->m_collection[i] || other.m_collection[i]);
            return *this;
        } // operator |=(...)

        /** Elementwise "and". */
        type& operator &=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] = (this->m_collection[i] && other.m_collection[i]);
            return *this;
        } // operator &=(...)

        /** Elementwise "exclusive or". */
        type& operator ^=(const type& other) noexcept
        {
            for (std::size_t i = 0; i < type::capacity; ++i) this->m_collection[i] = (this->m_collection[i] ^ other.m_collection[i]);
            return *this;
        } // operator ^=(...)

        /** Something clever taken from http://en.cppreference.com/w/cpp/language/operators. */
        friend type operator |(type left, const type& right) noexcept { left |= right; return left; }
        friend type operator &(type left, const type& right) noexcept { left &= right; return left; }
        friend type operator ^(type left, const type& right) noexcept { left ^= right; return left; }

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array<...>

    /** @brief Lists enumerable keys of \tparam t_enum_type. */
    template <typename t_enum_type>
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

        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            nlohmann::json j = self;
            return os << j;
        } // operator <<(...)
    }; // struct enum_array<...>

    /** Store as an object { ..., "<enum key>": value, ... }. */
    template <typename t_enum_type, typename t_value_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, t_value_type>& x) noexcept
    {
        j = { };
        t_value_type z { };
        for (const auto& pair : x)
        {
            if (pair.value() == z) continue; // Skip default values.
            j[detail::enum_parser<t_enum_type>::to_string(pair.key())] = pair.value();
        } // for (...)
    } // to_json(...)

    /** Store as an array [ ..., "<enum key>", ... ]. */
    template <typename t_enum_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, bool>& x) noexcept
    {
        std::vector<std::string> y { };
        y.reserve(enum_array<t_enum_type, bool>::capacity);
        for (t_enum_type value : x) y.push_back(detail::enum_parser<t_enum_type>::to_string(value));
        j = y;
    } // to_json(...)

    /** Store as an array [ ..., "<enum key>", ... ]. */
    template <typename t_enum_type>
    void to_json(nlohmann::json& j, const enum_array<t_enum_type, void>& x) noexcept
    {
        std::vector<std::string> y { };
        y.reserve(enum_array<t_enum_type, void>::capacity);
        for (t_enum_type value : x) y.push_back(detail::enum_parser<t_enum_type>::to_string(value));
        j = y;
    } // to_json(...)

    /** Unpack object { ..., "<enum key>": value, ... }. */
    template <typename t_enum_type, typename t_value_type>
    void from_json(const nlohmann::json& j, enum_array<t_enum_type, t_value_type>& x) noexcept
    {
        quiet_json q(j);
        using type = enum_array<t_enum_type, t_value_type>;
        using underlying_type = std::underlying_type_t<t_enum_type>;

        for (underlying_type k = type::first_index; k < type::past_the_last_index; ++k)
        {
            t_enum_type key = static_cast<t_enum_type>(k);
            t_value_type value { };
            if (q.optional(detail::enum_parser<t_enum_type>::to_string(key), value)) x[key] = value;
        }

        if (!q.good())
        {
            aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major,
                q.message(), __FUNCTION__, __LINE__);
            return;
        } // if (...)
    } // from_json(...)

    /** Unpack array [ ..., "<enum key>", ... ]. */
    template <typename t_enum_type>
    void from_json(const nlohmann::json& j, enum_array<t_enum_type, bool>& x) noexcept
    {
        quiet_json q(j);
        //using type = enum_array<t_enum_type, bool>;
        //using underlying_type = std::underlying_type_t<t_enum_type>;

        std::vector<std::string> str_vector { };
        if (q.interpret_as(str_vector))
        {
            for (const std::string& key_str : str_vector)
            {
                t_enum_type key { };
                if (detail::enum_parser<t_enum_type>::try_parse(key_str, key)) x[key] = true;
                else aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::minor,
                    std::string("Skipping unrecognized enum: ") + key_str + std::string("."), __FUNCTION__, __LINE__);
            }
        } // if (...)

        if (!q.good())
        {
            aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major,
                q.message(), __FUNCTION__, __LINE__);
            return;
        } // if (...)
    } // from_json(...)

    /** Unpack array [ ..., "<enum key>", ... ]. */
    template <typename t_enum_type>
    void from_json(const nlohmann::json& j, enum_array<t_enum_type, void>& x) noexcept
    {
        quiet_json q(j);
        //using type = enum_array<t_enum_type, void>;
        //using underlying_type = std::underlying_type_t<t_enum_type>;

        std::vector<std::string> str_vector{};
        if (q.interpret_as(str_vector))
        {
            /** @todo Check existing strings in \c str_vector against \c enum_array values. */
            if (str_vector.size() != x.size())
                aftermath::quiet_error::instance().push(
                    aftermath::not_an_error::runtime_error,
                    aftermath::severity_level::minor,
                    std::string("Size mismatch."), __FUNCTION__, __LINE__);
        } // if (...)

        if (!q.good())
        {
            aftermath::quiet_error::instance().push(
                aftermath::not_an_error::runtime_error,
                aftermath::severity_level::major,
                q.message(), __FUNCTION__, __LINE__);
            return;
        } // if (...)
    } // from_json(...)
} // namespace ropufu::aftermath

namespace std
{
    template <typename t_enum_type, typename t_data_type>
    std::string to_string(const ropufu::aftermath::enum_array<t_enum_type, t_data_type>& value) noexcept
    {
        nlohmann::json j = value;
        return j.dump();
    } // to_string(...)
} // namespace std

#endif // ROPUFU_AFTERMATH_ENUM_ARRAY_HPP_INCLUDED
