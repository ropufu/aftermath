
#ifndef ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED

#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"

#include "range_spacing.hpp"

#include <cmath>      // std::log10, std::pow
#include <cstddef>    // std::size_t
#include <functional> // std::hash
#include <initializer_list> // std::initializer_list
#include <iostream>   // std::ostream
#include <stdexcept>  // std::runtime_error
#include <string>     // std::string
#include <system_error> // std::error_code, std::errc
#include <vector>     // std::vector

namespace ropufu::aftermath::algebra
{
    namespace detail
    {
        template <typename t_value_type, typename t_container_type>
        struct range_container
        {
            using type = range_container<t_value_type, t_container_type>;
            using value_type = t_value_type;
            using container_type = t_container_type;

            static container_type make_empty(std::size_t count = 0) noexcept;
            static container_type make_init(std::initializer_list<value_type>) noexcept;
            static void shrink(container_type& container) noexcept;
        }; // struct range_container

        template <typename t_value_type>
        struct range_container<t_value_type, std::vector<t_value_type>>
        {
            using type = range_container<t_value_type, std::vector<t_value_type>>;
            using value_type = t_value_type;
            using container_type = std::vector<t_value_type>;

            static container_type make_empty(std::size_t count = 0) noexcept { return container_type(count); }
            static container_type make_init(std::initializer_list<value_type> data) noexcept { return container_type(data); }
            static void shrink(container_type& container) noexcept { container.shrink_to_fit(); };
        }; // struct range_container<...>
    } // namespace detail

    template <typename t_value_type>
    struct range;
    template <typename t_value_type>
    void to_json(nlohmann::json& j, const range<t_value_type>& x) noexcept;
    template <typename t_value_type>
    void from_json(const nlohmann::json& j, range<t_value_type>& x);

    /** @brief Inspired by MATLAB's linspace function. */
    template <typename t_value_type>
    struct range
    {
        using type = range<t_value_type>;
        using value_type = t_value_type;

        // ~~ Json names ~~
        static constexpr char jstr_from[] = "from";
        static constexpr char jstr_to[] = "to";

    private:
        value_type m_from = {};
        value_type m_to = {};

    public:
        range() noexcept { }
        
        range(const value_type& from, const value_type& to) noexcept
            : m_from(from), m_to(to)
        {
        } // range(...)

        range(const nlohmann::json& j, std::error_code& ec) noexcept
        {
            if (j.is_array())
            {
                std::vector<value_type> range_pair = {};
                aftermath::noexcept_json::as(j, range_pair, ec);
                if (ec.value() != 0) return;
                if (range_pair.size() != 2) // Range should be a vector with two entries.
                {
                    ec = std::make_error_code(std::errc::bad_message);
                    return;
                } // if (...)
                this->m_from = range_pair.front();
                this->m_to = range_pair.back();
            } // if (...)
            else
            {
                aftermath::noexcept_json::required(j, type::jstr_from, this->m_from, ec);
                aftermath::noexcept_json::required(j, type::jstr_to, this->m_to, ec);
            } // else (...)
        } // range(...)

        const value_type& from() const noexcept { return this->m_from; }
        const value_type& to() const noexcept { return this->m_to; }

        template <typename t_container_type>
        void explode(t_container_type& container, std::size_t count) const noexcept
        {
            linear_spacing<value_type> lin_spacing {};
            this->explode(container, count, lin_spacing);
        } // explode(...)

        template <typename t_container_type, typename t_spacing_type>
        void explode(t_container_type& container, std::size_t count, const t_spacing_type& spacing) const noexcept
        {
            using helper_type = detail::range_container<value_type, t_container_type>;
            using intermediate_type = typename t_spacing_type::intermediate_type;

            switch (count)
            {
                case 0: container = helper_type::make_empty(); return;
                case 1: container = helper_type::make_init({ this->m_from }); return;
                case 2: container = helper_type::make_init({ this->m_from, this->m_to }); return;
            } // switch (...)

            container = helper_type::make_empty(count);

            intermediate_type f_from = spacing.forward_transform(this->m_from);
            intermediate_type f_to = spacing.forward_transform(this->m_to);
            intermediate_type f_range = f_to - f_from;

            std::size_t i = 0;
            bool is_first = true;

            for (value_type& x : container)
            {
                bool is_last = (i == count - 1);

                if (is_first) x = this->m_from;
                else if (is_last) x = this->m_to;
                else
                {
                    intermediate_type f_step = (i * f_range) / (count - 1);
                    x = spacing.backward_transform(f_from + f_step);
                } // for (...)
                ++i;
                is_first = false;
            } // for (...)
            helper_type::shrink(container);
        } // explode(...)

        /** Checks if the two objects are equal. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_from == other.m_from &&
                this->m_to == other.m_to;
        } // operator ==(...)

        /** Checks if the two objects are not equal. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

        /** @brief Output to a stream. */
        friend std::ostream& operator <<(std::ostream& os, const type& self) noexcept
        {
            return os << self.m_from << "--" << self.m_to;
        } // operator <<(...)
    }; // struct range

    // ~~ Json name definitions ~~
    template <typename t_value_type> constexpr char range<t_value_type>::jstr_from[];
    template <typename t_value_type> constexpr char range<t_value_type>::jstr_to[];
    
    template <typename t_value_type>
    void to_json(nlohmann::json& j, const range<t_value_type>& x) noexcept
    {
        using type = range<t_value_type>;

        j = nlohmann::json{
            {type::jstr_from, x.from()},
            {type::jstr_to, x.to()}
        };
    } // to_json(...)

    template <typename t_value_type>
    void from_json(const nlohmann::json& j, range<t_value_type>& x)
    {
        using type = range<t_value_type>;
        std::error_code ec {};
        x = type(j, ec);
        if (ec.value() != 0) throw std::runtime_error("Parsing <range> failed: " + j.dump());
    } // from_json(...)
} // namespace ropufu::aftermath::algebra

namespace std
{
    template <typename t_value_type>
    struct hash<ropufu::aftermath::algebra::range<t_value_type>>
    {
        using argument_type = ropufu::aftermath::algebra::range<t_value_type>;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            std::hash<t_value_type> value_hash {};
            return
                (value_hash(x.from()) << 4) ^ 
                (value_hash(x.to()));
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_RANGE_HPP_INCLUDED
