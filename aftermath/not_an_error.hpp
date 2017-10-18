
#ifndef ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED
#define ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED

#include <cstddef> // std::size_t
#include <stack> // std::stack
#include <string> // std::string, std::to_string

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

        /** Brief list of severity level. */
        enum struct severity_level
        {
            not_at_all, // Not severe.
            negligible, // Negligible, may usually be ignored.
            minor,      // Minor, but may cause unpredicted behavior.
            major,      // Major, will very likely cause unpredicted behavior.
            fatal       // Oh no!!
        };

        /** Descriptor for errors/warnings/messages. */
        struct quiet_error_descriptor
        {
        private:
            not_an_error m_error_code = not_an_error::all_good; // Errors code.
            severity_level m_severity = severity_level::not_at_all; // Severity level of the error/warning.
            std::string m_description = ""; // Detailed error/warning message.
            std::string m_caller_function_name = ""; // The function name that issued the error/warning.
            std::size_t m_caller_line_number = 0; // The line number where the error/warning was issued.

        public:
            quiet_error_descriptor() noexcept { }

            quiet_error_descriptor(
                not_an_error error_code, severity_level severity, const std::string& description,
                const std::string& caller_function_name, std::size_t caller_line_number) noexcept
                : m_error_code(error_code), m_severity(severity), m_description(description),
                m_caller_function_name(caller_function_name), m_caller_line_number(caller_line_number)
            {

            }

            not_an_error error_code() const noexcept { return this->m_error_code; }
            severity_level severity() const noexcept { return this->m_severity; }
            std::string description() const noexcept { return this->m_description; }
            std::string caller_function_name() const noexcept { return this->m_caller_function_name; }
            std::size_t caller_line_number() const noexcept { return this->m_caller_line_number; }
        };

        /** @brief Singleton type for storing quiet errors.
         *  @remark Singleton structure taken from https://stackoverflow.com/questions/11711920
         */
        struct quiet_error
        {
            using type = quiet_error;

        private:
            bool m_is_good = true; // Indicates if no errors have been recorded.
            std::stack<quiet_error_descriptor> m_errors; // Stores errors/warnings/messages.

        protected:
            quiet_error() noexcept : m_errors() { }
            ~quiet_error() noexcept { }

        public:
            /** Indicates if no errors with severity other than \c severity_level::not_at_all have been recorded. */
            bool good() const noexcept { return this->m_is_good; }

            /** Indicates if no warnings or errors have been recorded. */
            bool empty() const noexcept { return this->m_errors.empty(); }
            
            /** Adds a simple message/warning to the stack. */
            void push(const std::string& message) noexcept
            {
                this->m_errors.emplace(not_an_error::all_good, severity_level::not_at_all, message, "", 0);
            }
            
            /** @brief Adds an error to the stack.
             *  @param caller_function_name Typically \c __FUNCTION__.
             *  @param caller_line_number Typically \c __LINE__.
             */
            void push(not_an_error error, severity_level severity,
                const std::string& message = "",
                const std::string& caller_function_name = "", std::size_t caller_line_number = 0) noexcept
            {
                if (severity != severity_level::not_at_all) this->m_is_good = false;
                this->m_errors.emplace(error, severity, message, caller_function_name, caller_line_number);
            }
            
            /** Reads the latest error/warning and removes it from the stack. */
            quiet_error_descriptor pop() noexcept
            {
                if (this->m_errors.empty()) return { };

                quiet_error_descriptor result = this->m_errors.top();
                this->m_errors.pop();
                if (this->m_errors.empty()) this->m_is_good = true;

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
    }
}

namespace std
{
    std::string to_string(ropufu::aftermath::not_an_error value)
    {
        switch (value)
        {
        case ropufu::aftermath::not_an_error::all_good: return "not an error";
        case ropufu::aftermath::not_an_error::logic_error: return "logic error";
        case ropufu::aftermath::not_an_error::invalid_argument: return "invalid argument";
        case ropufu::aftermath::not_an_error::domain_error: return "domain error";
        case ropufu::aftermath::not_an_error::length_error: return "length error";
        case ropufu::aftermath::not_an_error::out_of_range: return "out of range";
        case ropufu::aftermath::not_an_error::runtime_error: return "runtime error";
        case ropufu::aftermath::not_an_error::range_error: return "range error";
        case ropufu::aftermath::not_an_error::overflow_error: return "overflow error";
        case ropufu::aftermath::not_an_error::underflow_error: return "underflow error";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
    
    std::string to_string(ropufu::aftermath::severity_level value)
    {
        switch (value)
        {
        case ropufu::aftermath::severity_level::not_at_all: return "log";
        case ropufu::aftermath::severity_level::negligible: return "negligible";
        case ropufu::aftermath::severity_level::minor: return "minor";
        case ropufu::aftermath::severity_level::major: return "major";
        case ropufu::aftermath::severity_level::fatal: return "fatal";
        default: return std::to_string(static_cast<std::size_t>(value));
        }
    };
}

#endif // ROPUFU_AFTERMATH_NOT_AN_ERROR_HPP_INCLUDED
