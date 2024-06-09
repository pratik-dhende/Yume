#pragma once

#include <windows.h>
#include <type_traits>
#include <string>
#include <comdef.h>
#include <system_error>
#include <sstream>

namespace Yume
{
    // Type utilities

    template<typename E>
    constexpr std::underlying_type_t<E> toUType(const E& enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    std::wstring ansiToWString(const std::string& str);

    // Exception utitlies

    class Exception
    {
    public:
        Exception(const std::wstring& filename, const unsigned int lineNumber)
            : m_filename(filename), m_lineNumber(lineNumber)
        { }

        virtual std::wstring toWString() const = 0;

    protected:
        std::wstring m_filename;
        unsigned int m_lineNumber;
    };

    class Win32Exception : public Exception
    {
    public:
        Win32Exception(const std::wstring& filename, const unsigned int lineNumber)
            : Exception(filename, lineNumber)
        { }

        std::wstring toWString() const override
        {   
            std::wstringstream wss;

            const DWORD error = GetLastError();
            const std::wstring errorMsg = ansiToWString(std::system_category().message(error));
            
            wss << errorMsg << "\nFile: " << m_filename << " at line: " << m_lineNumber;

            return wss.str();
        }
    };

    class DXException : public Exception
    {
    public:
        DXException(const HRESULT hr, const std::wstring& filename, const unsigned int lineNumber)
            : Exception(filename, lineNumber), m_errorCode(hr)
        {}

        std::wstring toWString() const override
        {   
            std::wstringstream wss;

            const _com_error error(m_errorCode);
            const std::wstring errorMsg = error.ErrorMessage();

            wss << errorMsg << "\nFile: " << m_filename << " at line: " << m_lineNumber;

            return wss.str();
        }

    private:
        HRESULT m_errorCode;
    };
}

#ifndef YM_THROW_IF_FAILED_WIN32_EXCEPTION
#define YM_THROW_IF_FAILED_WIN32_EXCEPTION(result)                    \
{                                                                     \
   if (!result) { throw Win32Exception(__FILEW__, __LINE__); }                           \
}
#endif

#ifndef YM_THROW_IF_FAILED_DX_EXCEPTION
#define YM_THROW_IF_FAILED_DX_EXCEPTION(result)                       \
{                                                                     \
    if(FAILED(result)) { throw DXException(__FILEW__, __LINE__); }                        \
}
#endif