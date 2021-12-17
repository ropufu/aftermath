
#ifndef ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "../noexcept_json.hpp"
#endif

#include "../concepts.hpp"
#include "interval.hpp"
#include "interval_spacing.hpp"

#include <concepts>    // std::same_as
#include <cstddef>     // std::size_t
#include <functional>  // std::hash
#include <ranges>      // std::ranges::...
#include <stdexcept>   // std::runtime_error
#include <string_view> // std::string_view

#ifdef ROPUFU_TMP_TYPENAME
#undef ROPUFU_TMP_TYPENAME
#endif
#ifdef ROPUFU_TMP_TEMPLATE_SIGNATURE
#undef ROPUFU_TMP_TEMPLATE_SIGNATURE
#endif
#define ROPUFU_TMP_TYPENAME interval_based_vector<t_spacing_type>
#define ROPUFU_TMP_TEMPLATE_SIGNATURE template <ropufu::spacing t_spacing_type>


namespace ropufu::aftermath::algebra
{
    /** @brief A range-based representation of a collection.
     *  @example {"range": [1, 5], "spacing": "linear", "count": 3} is a range-based representation of [1, 3, 5].
     */
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct interval_based_vector;

#ifndef ROPUFU_NO_JSON
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void to_json(nlohmann::json& j, const ROPUFU_TMP_TYPENAME& x) noexcept;
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    void from_json(const nlohmann::json& j, ROPUFU_TMP_TYPENAME& x);
#endif

    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct interval_based_vector
    {
        using type = ROPUFU_TMP_TYPENAME;
        using spacing_type = t_spacing_type;
        
        using value_type = typename spacing_type::value_type;
        using interval_type = interval<value_type>;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_range = "range";
        static constexpr std::string_view jstr_spacing = "spacing";
        static constexpr std::string_view jstr_count = "count";

#ifndef ROPUFU_NO_JSON
        friend ropufu::noexcept_json_serializer<type>;
#endif
        friend std::hash<type>;
        
    private:
        interval_type m_range = {};
        spacing_type m_spacing = {};
        std::size_t m_count = 0;

    public:
        interval_based_vector() noexcept = default;

        const interval_type& range() const noexcept { return this->m_range; }
        void set_range(const interval_type& value) noexcept { this->m_range = value; }

        const spacing_type& spacing() const noexcept { return this->m_spacing; }
        void set_spacing(const spacing_type& value) noexcept { this->m_spacing = value; }

        std::size_t count() const noexcept { return this->m_count; }
        void set_count(std::size_t value) noexcept { this->m_count = value; }
            
        template <ropufu::push_back_container t_container_type>
            requires std::same_as<std::ranges::range_value_t<t_container_type>, value_type>
        void explode(t_container_type& container) const noexcept
        {
            using intermediate_type = typename t_spacing_type::intermediate_type;

            switch (this->m_count)
            {
                case 0: container = {}; return;
                case 1: container = { this->m_range.from() }; return;
                case 2: container = { this->m_range.from(), this->m_range.to() }; return;
            } // switch (...)

            container.clear();

            intermediate_type f_from = this->m_spacing.forward_transform(this->m_range.from());
            intermediate_type f_to = this->m_spacing.forward_transform(this->m_range.to());
            intermediate_type f_diameter = f_to - f_from;
            intermediate_type sentinel = static_cast<intermediate_type>(this->m_count - 1);
            
            container.push_back(this->m_range.from()); // First value is always the left end-point.
            for (intermediate_type i = 1; i < sentinel; ++i)
            {
                intermediate_type f_step = (i * f_diameter) / sentinel;
                value_type x = this->m_spacing.backward_transform(f_from + f_step);

                container.push_back(x);
            } // for (...)
            container.push_back(this->m_range.to()); // Last value is always the right end-point.
        } // explode(...)

        /** Checks if the two objects are equal. */
        bool operator ==(const type& other) const noexcept
        {
            return
                this->m_range == other.m_range &&
                this->m_spacing == other.m_spacing &&
                this->m_count == other.m_count;
        } // operator ==(...)

        /** Checks if the two objects are not equal. */
        bool operator !=(const type& other) const noexcept { return !this->operator ==(other); }

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            j = nlohmann::json{
                {type::jstr_range, x.m_range},
                {type::jstr_spacing, x.m_spacing},
                {type::jstr_count, x.m_count}
            };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <interval_based_vector> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct interval_based_vector<...>
} // namespace ropufu::aftermath::algebra

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct noexcept_json_serializer<ropufu::aftermath::algebra::ROPUFU_TMP_TYPENAME>
    {
        using result_type = ropufu::aftermath::algebra::ROPUFU_TMP_TYPENAME;
        
        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (!noexcept_json::required(j, result_type::jstr_range, x.m_range)) return false;
            if (!noexcept_json::required(j, result_type::jstr_spacing, x.m_spacing)) return false;
            if (!noexcept_json::required(j, result_type::jstr_count, x.m_count)) return false;

            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

namespace std
{
    ROPUFU_TMP_TEMPLATE_SIGNATURE
    struct hash<ropufu::aftermath::algebra::ROPUFU_TMP_TYPENAME>
    {
        using argument_type = ropufu::aftermath::algebra::ROPUFU_TMP_TYPENAME;
        using result_type = std::size_t;

        result_type operator ()(const argument_type& x) const noexcept
        {
            result_type result = 0;
            constexpr std::size_t parameter_dim = 3;
            constexpr result_type total_width = sizeof(result_type);
            constexpr result_type width = total_width / (argument_type::parameter_dim);
            constexpr result_type shift = (width == 0 ? 1 : width);

            std::hash<typename argument_type::interval_type> range_hash {};
            std::hash<typename argument_type::spacing_type> spacing_hash {};
            std::hash<std::size_t> count_hash {};

            result ^= (range_hash(x.m_range) << ((shift * 0) % total_width));
            result ^= (spacing_hash(x.m_spacing) << ((shift * 1) % total_width));
            result ^= (count_hash(x.m_count) << ((shift * 2) % total_width));

            return result;
        } // operator ()(...)
    }; // struct hash<...>
} // namespace std

#endif // ROPUFU_AFTERMATH_ALGEBRA_INTERVAL_BASED_VECTOR_HPP_INCLUDED
