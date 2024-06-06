#pragma once

#include <windows.h>
#include <type_traits>
#include <string>

namespace Yume
{
    // Type utilities

    template<typename E>
    constexpr std::underlying_type_t<E> toUType(const E& enumerator) noexcept
    {
        return static_cast<std::underlying_type_t<E>>(enumerator);
    }

    std::wstring ansiToWString(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    // Exception utitlies

    class DXException
    {
    public:
        DXException() = default;
        DXException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);
        
        std::wstring toString() const;
    
    public:
        HRESULT m_errorCode = S_OK;

        std::wstring m_functionName;
        std::wstring m_filename;

        int m_lineNumber = -1;
    };
}

#ifndef YM_THROW_IF_FAILED
#define YM_THROW_IF_FAILED(x)                                         \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = Yume::ansiToWString(__FILE__);                 \
    if(FAILED(hr__)) { throw DXException(hr__, L#x, wfn, __LINE__); } \
}
#endif