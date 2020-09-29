#ifndef H_XD_EXCEPTION
#define H_XD_EXCEPTION

#include <stdexcept>
#include <string>

namespace xd
{
    class exception : public std::exception
    {
    public:
        exception()
        {
        }

        exception(const std::string& message)
            : m_message(message)
        {
        }

        virtual ~exception() noexcept
        {
        }

        virtual const char *what() const noexcept
        {
            if (m_message.length() != 0)
                return m_message.c_str();
            else
                return "unknown xd::exception";
        }

    private:
        std::string m_message;
    };
}

#endif
