
#ifndef ROPUFU_AFTERMATH_ON_ERROR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_ON_ERROR_HPP_INCLUDED

#include <string>  // std::string
#include <system_error> // std::error_code, std::errc, std::make_error_code
#include <utility> // std::forward

namespace ropufu::aftermath
{
    namespace detail
    {
        /** @todo Figure out what to do with messages. */
        void on_error(std::error_code& ec, std::errc code, std::string&& /*message*/) noexcept
        {
            ec = std::make_error_code(code);
        } // on_error(...)

        /** @todo Figure out what to do with messages. */
        template <typename t_return_type>
        const t_return_type& on_error(std::error_code& ec, std::errc code, std::string&& /*message*/, const t_return_type& result) noexcept
        {
            ec = std::make_error_code(code);
            return result;
        } // on_error(...)

        /** @todo Figure out what to do with messages. */
        template <typename t_return_type>
        const t_return_type& on_error(std::error_code& ec, std::errc code, std::string&& /*message*/, t_return_type&& result) noexcept
        {
            ec = std::make_error_code(code);
            return result;
            //return std::forward(result);
        } // on_error(...)
    } // namespace detail
} // namespace ropufu::aftermath

#endif // ROPUFU_AFTERMATH_ON_ERROR_HPP_INCLUDED
