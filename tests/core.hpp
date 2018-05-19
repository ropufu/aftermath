
#include <nlohmann/json.hpp>

#include <sstream> // std::ostringstream

namespace ropufu::test_aftermath
{
    template <typename t_type>
    bool test_json_round_trip(const t_type& x) noexcept
    {
        nlohmann::json j = x;
        t_type y = j;
        return x == y;
    } // test_json_round_trip(...)

    template <typename t_type>
    bool test_ostream(const t_type& x, const t_type& y) noexcept
    {
        std::ostringstream ssx { };
        std::ostringstream ssy { };

        ssx << x;
        ssy << y;
        return ssx.str() == ssy.str();
    }
} // namespace ropufu::test_aftermath
