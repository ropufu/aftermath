
#ifndef ROPUFU_AFTERMATH_KEY_VALUE_PAIR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_KEY_VALUE_PAIR_HPP_INCLUDED

namespace ropufu::aftermath
{
    namespace detail
    {
        /** Auxiliary structure to help iterate through \c enum_array. */
        template <typename t_key_type, typename t_value_ptr_type>
        struct key_value_pair { };

        /** Auxiliary structure to help iterate through \c enum_array. */
        template <typename t_key_type, typename t_value_type>
        struct key_value_pair<t_key_type, t_value_type*>
        {
            using type = key_value_pair<t_key_type, t_value_type*>;
            using key_type = t_key_type;
            using value_type = t_value_type;

        private:
            key_type m_key = { };
            value_type* m_value_ptr = nullptr;

        public:
            key_value_pair(key_type key, value_type* value_ptr) noexcept
                : m_key(key), m_value_ptr(value_ptr)
            {
            } // key_value_pair(...)

            key_type key() const noexcept { return this->m_key; }

            value_type& value() noexcept { return *(this->m_value_ptr); }

            const value_type& value() const noexcept { return *(this->m_value_ptr); }
        }; // struct key_value_pair<...>

        /** Auxiliary structure to help iterate through \c enum_array. */
        template <typename t_key_type, typename t_value_type>
        struct key_value_pair<t_key_type, const t_value_type*>
        {
            using type = key_value_pair<t_key_type, const t_value_type*>;
            using key_type = t_key_type;
            using value_type = t_value_type;

        private:
            key_type m_key = { };
            const value_type* m_value_ptr = nullptr;

        public:
            key_value_pair(key_type key, const value_type* value_ptr) noexcept
                : m_key(key), m_value_ptr(value_ptr)
            {
            } // key_value_pair(...)

            key_type key() const noexcept { return this->m_key; }

            const value_type& value() const noexcept { return *(this->m_value_ptr); }
        }; // struct key_value_pair<...>
    } // namespace detail
} // ropufu::aftermath

#endif // ROPUFU_AFTERMATH_KEY_VALUE_PAIR_HPP_INCLUDED
