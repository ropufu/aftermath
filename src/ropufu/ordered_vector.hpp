
#ifndef ROPUFU_SEQUENTIAL_INTERMITTENT_ORDERED_VECTOR_HPP_INCLUDED
#define ROPUFU_SEQUENTIAL_INTERMITTENT_ORDERED_VECTOR_HPP_INCLUDED

#ifndef ROPUFU_NO_JSON
#include <nlohmann/json.hpp>
#include "noexcept_json.hpp"
#endif

#include "algebra/interval.hpp"
#include "algebra/interval_spacing.hpp"

#include <algorithm>   // std::sort
#include <concepts>    // std::totally_ordered
#include <cstddef>     // std::size_t, std::ptrdiff_t
#include <memory>      // std::allocator
#include <stdexcept>   // std::logic_error
#include <string>      // std::string
#include <string_view> // std::string_view
#include <utility>     // std::move
#include <vector>      // std::vector

namespace ropufu
{
    /** @brief A vector that supports ordering, and may accomodate range/spacing representation.
     *  @example [1, 2, 3.4, 3, 0] is an \c ordered_vector<double>.
     *  @example {"range": [1, 5], "spacing": "linear", "count": 3} is a representation of [1, 3, 5].
     *  @note The collection is not(!) sorted automatically.
     */
    template <std::totally_ordered t_value_type, typename t_allocator_type = std::allocator<t_value_type>>
    struct ordered_vector;

#ifndef ROPUFU_NO_JSON
    template <std::totally_ordered t_value_type, typename t_allocator_type>
    void to_json(nlohmann::json& j, const ordered_vector<t_value_type, t_allocator_type>& x) noexcept;
    template <std::totally_ordered t_value_type, typename t_allocator_type>
    void from_json(const nlohmann::json& j, ordered_vector<t_value_type, t_allocator_type>& x);
#endif

    template <std::totally_ordered t_value_type, typename t_allocator_type>
    struct ordered_vector : public std::vector<t_value_type, t_allocator_type>
    {
        using type = ordered_vector<t_value_type, t_allocator_type>;
        using base_type = std::vector<t_value_type, t_allocator_type>;
        using value_type = t_value_type;
        using allocator_type = t_allocator_type;

        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using interval_type = ropufu::aftermath::algebra::interval<t_value_type>;

        // ~~ Json names ~~
        static constexpr std::string_view jstr_range = "range";
        static constexpr std::string_view jstr_spacing = "spacing";
        static constexpr std::string_view jstr_count = "count";

        friend ropufu::noexcept_json_serializer<type>;

    private:
        interval_type m_range = {};
        std::string m_spacing_name = {};

    public:
        using base_type::base_type;

        template <ropufu::spacing t_spacing_type>
        ordered_vector(const interval_type& range, std::size_t count, const t_spacing_type& spacing)
            : m_range(range), m_spacing_name(t_spacing_type::name)
        {
            ropufu::aftermath::algebra::explode(range, *this, count, spacing);
        } // ordered_vector(...)

        bool is_range_based() const noexcept { return !this->m_spacing_name.empty(); }

        explicit operator base_type() const noexcept
        {
            return base_type(static_cast<const base_type*>(this));
        } // operator base_type(...)

        void sort()
        {
            std::sort(this->begin(), this->end());
        } // sort(...)

        template <typename t_comparer_type>
        void sort(t_comparer_type comparer)
        {
            std::sort(this->begin(), this->end(), comparer);
        } // sort(...)

        void fill(const value_type& value) noexcept
        {
            for (value_type& x : *this) x = value;
        } // fill(...)

        bool contains(const value_type& value) noexcept
        {
            for (const value_type& x : *this) if (x == value) return true;
            return false;
        } // contains(...)

#ifndef ROPUFU_NO_JSON
        friend void to_json(nlohmann::json& j, const type& x) noexcept
        {
            if (!x.is_range_based()) j = static_cast<const base_type&>(x);
            else
                j = nlohmann::json{
                    {type::jstr_range, x.m_range},
                    {type::jstr_spacing, x.m_spacing_name},
                    {type::jstr_count, x.size()}
                };
        } // to_json(...)

        friend void from_json(const nlohmann::json& j, type& x)
        {
            if (!ropufu::noexcept_json::try_get(j, x))
                throw std::runtime_error("Parsing <ordered_vector> failed: " + j.dump());
        } // from_json(...)
#endif
    }; // struct ordered_vector
} // namespace ropufu

#ifndef ROPUFU_NO_JSON
namespace ropufu
{
    template <std::totally_ordered t_value_type, typename t_allocator_type>
    struct noexcept_json_serializer<ropufu::ordered_vector<t_value_type, t_allocator_type>>
    {
        using result_type = ropufu::ordered_vector<t_value_type, t_allocator_type>;
        
        using linear_spacing_type = ropufu::aftermath::algebra::linear_spacing<t_value_type>;
        using logarithmic_spacing = ropufu::aftermath::algebra::logarithmic_spacing<t_value_type>;
        using exponential_spacing = ropufu::aftermath::algebra::exponential_spacing<t_value_type>;

        static bool try_get(const nlohmann::json& j, result_type& x) noexcept
        {
            if (j.is_array())
            {
                typename result_type::base_type& y = x; //static_cast<typename result_type::base_type&>(x);
                if (!noexcept_json::try_get(j, y)) return false;
            } // if (...)
            else
            {
                std::size_t count = 0;
                if (!noexcept_json::required(j, result_type::jstr_range, x.m_range)) return false;
                if (!noexcept_json::required(j, result_type::jstr_spacing, x.m_spacing_name)) return false;
                if (!noexcept_json::required(j, result_type::jstr_count, count)) return false;

                if (x.m_spacing_name == linear_spacing_type::name)
                    ropufu::aftermath::algebra::explode(x.m_range, x, count, linear_spacing_type{});
                else if (x.m_spacing_name == logarithmic_spacing::name)
                    ropufu::aftermath::algebra::explode(x.m_range, x, count, logarithmic_spacing{});
                else if (x.m_spacing_name == exponential_spacing::name)
                    ropufu::aftermath::algebra::explode(x.m_range, x, count, exponential_spacing{});
                else return false;
            } // if (...)
            return true;
        } // try_get(...)
    }; // struct noexcept_json_serializer<...>
} // namespace ropufu
#endif

#endif // ROPUFU_SEQUENTIAL_INTERMITTENT_ORDERED_VECTOR_HPP_INCLUDED
