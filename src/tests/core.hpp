
#ifndef ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED

#include <nlohmann/json.hpp>

#include <sstream> // std::ostringstream

namespace ropufu::test_aftermath
{
    template <typename t_type>
    bool test_json_round_trip(const t_type& x) noexcept
    {
        try
        {
            nlohmann::json j = x;
            t_type y = j;
            return x == y;
        }
        catch (...) { return false; }
    } // test_json_round_trip(...)

    template <typename t_type>
    bool test_ostream(const t_type& x, const t_type& y) noexcept
    {
        try
        {
            std::ostringstream ssx { };
            std::ostringstream ssy { };

            ssx << x;
            ssy << y;
            return ssx.str() == ssy.str();
        }
        catch (...) { return false; }
    } // test_ostream(...)
} // namespace ropufu::test_aftermath

#endif // ROPUFU_AFTERMATH_TESTS_CORE_HPP_INCLUDED
