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

    UINT nextMultiple256(UINT byteSize)
    {
        /*
            Constant buffers must be a multiple of the minimum hardware
            allocation size (usually 256 bytes).  So round up to nearest
            multiple of 256.  We do this by adding 255 and then masking off
            the lower 2 bytes which store all bits < 256.
            Example: Suppose byteSize = 300.
            (300 + 255) & ~255
            555 & ~255
            0x022B & ~0x00ff
            0x022B & 0xff00
            0x0200
            512
        */
        return (byteSize + 255) & ~255;
    }
}