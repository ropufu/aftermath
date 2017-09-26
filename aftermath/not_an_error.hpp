
#ifndef ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED

#include <cstddef>
#include <stack>
#include <string>

namespace ropufu
{
    namespace aftermath
    {
        /** Brief list of error codes. */
        enum struct not_an_error
        {
            all_good,         // Indicates no error.
            logic_error,      // Indicates violations of logical preconditions or class invariants.
            invalid_argument, // Reports invalid arguments.
            domain_error,     // Reports domain errors.
            length_error,     // Reports attempts to exceed maximum allowed size.
            out_of_range,     // Reports arguments outside of expected range.
            runtime_error,    // Indicates conditions only detectable at run time.
            range_error,      // Reports range errors in internal computations.
            overflow_error,   // Reports arithmetic overflows.
            underflow_error   // Reports arithmetic underflows.
        };

        /** @brief Singleton type for storing quiet errors.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct quiet_error
        {
            typedef quiet_error type;

        private:
            bool m_is_good = true; // Indicates if no errors have been recorded.
            std::stack<not_an_error> m_errors; // Stores errors codes.
            std::stack<std::string> m_messages; // Stores detailed error/warning messages.
            std::stack<std::string> m_function_names; // Stores the function name that issued the error/warning.
            std::stack<std::size_t> m_lines; // Stores the line number where the error/warning was issued.

        protected:
            quiet_error() noexcept : m_errors(), m_messages(), m_function_names(), m_lines() { }
            ~quiet_error() noexcept {  }

        public:
            /** Indicates if no errors have been recorded. */
            bool good() const noexcept { return this->m_is_good; }

            /** Indicates if no warnings or errors have been recorded. */
            bool empty() const noexcept { return this->m_errors.empty(); }
            
            /** Adds a simple message/warning to the stack. */
            void push(const std::string& message) noexcept
            {
                this->m_errors.push(not_an_error::all_good);
                this->m_messages.push(message);
                this->m_function_names.push("");
                this->m_lines.push(0);
            }
            
            /** Adds an error to the stack. */
            void push(not_an_error error, const std::string& message = "", const std::string& function_name = "", std::size_t line = 0) noexcept
            {
                if (error != not_an_error::all_good) this->m_is_good = false;
                this->m_errors.push(error);
                this->m_messages.push(message);
                this->m_function_names.push(function_name);
                this->m_lines.push(line);
            }
            
            /** Reads the latest error/warning and removes it from the stack. */
            not_an_error pop() noexcept { std::string message; std::string function_name; std::size_t line; return this->pop(message, function_name, line); }

            /** Reads the latest error/warning and removes it from the stack. */
            not_an_error pop(std::string& message) noexcept { std::string function_name; std::size_t line; return this->pop(message, function_name, line); }

            /** Reads the latest error/warning and removes it from the stack. */
            not_an_error pop(std::string& message, std::string& function_name, std::size_t& line) noexcept
            {
                if (this->m_errors.empty()) return not_an_error::all_good;

                not_an_error result = this->m_errors.top();
                message = this->m_messages.top();
                function_name = this->m_function_names.top();
                line = this->m_lines.top();

                this->m_lines.pop();
                this->m_function_names.pop();
                this->m_messages.pop();
                this->m_errors.pop();

                return result;
            }
            
            /** The only instance of this type. */
            static type& instance()
            {
                // Since it's a static variable, if the class has already been created, it won't be created again.
                // Note: it is thread-safe since C++11.
                static type s_instance;

                // Return a reference to our instance.
                return s_instance;
            }

            // ~~ Delete copy and move constructors and assign operators ~~
            quiet_error(const type&) = delete; // Copy constructor.
            quiet_error(type&&) = delete; // Move constructor.
            type& operator =(const type&) = delete; // Copy assign.
            type& operator =(type&&) = delete; // Move assign.
        };

        // /** Another possible implementation of quite error returns. */
        // template <typename t_return_type = void>
        // struct quiet_return
        // {
        //     typedef quiet_return<t_return_type> type;
        //     typedef t_return_type return_type;

        // private:
        //     return_type m_return_value;
        //     not_an_error m_error = not_an_error::all_good;

        // public:
        //     quiet_return(const return_type& return_value) noexcept
        //         : m_return_value(return_value)
        //     {
        //     }

        //     quiet_return(return_type&& return_value) noexcept
        //         : m_return_value(return_value)
        //     {
        //     }

        //     quiet_return(const return_type& return_value, not_an_error error) noexcept
        //         : m_return_value(return_value), m_error(error)
        //     {
        //     }

        //     quiet_return(return_type&& return_value, not_an_error error) noexcept
        //         : m_return_value(return_value), m_error(error)
        //     {
        //     }

        //     operator return_type() const noexcept { return this->m_return_value; }

        //     const return_type& value() const noexcept { return this->m_return_value; }
        //     not_an_error error() const noexcept { return this->m_error; }
        // };

        // template <>
        // struct quiet_return<void>
        // {
        //     typedef quiet_return<void> type;
        //     typedef void return_type;

        // private:
        //     not_an_error m_error = not_an_error::all_good;

        // public:
        //     quiet_return(not_an_error error) noexcept
        //         : m_error(error)
        //     {
        //     }

        //     operator not_an_error() const noexcept { return this->m_error; }

        //     void value() const noexcept { }
        //     not_an_error error() const noexcept { return this->m_error; }
        // };
    }
}

#endif // ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED
