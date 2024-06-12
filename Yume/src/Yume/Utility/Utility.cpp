#include "ympch.h"

#include "Utility.h"

namespace Yume
{   
    std::wstring ansiToWString(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    std::string wStringToAnsi(const std::wstring& wstr)
    {   
        // TODO: Better alternative as this conversion can lose some characters and we cannot use wstring_convert as it is deprecated and removed in C++26
        return std::string(wstr.begin(), wstr.end());
    }
}