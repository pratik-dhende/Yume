#include "ympch.h"

#include "Utility.h"

namespace Yume
{   
    std::wstring ansiToWString(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }
}