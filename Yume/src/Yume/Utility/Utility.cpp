#include "ympch.h"

#include "Utility.h"

namespace Yume
{
    DXException::DXException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber)
        : m_errorCode(hr), m_functionName(functionName), m_filename(filename), m_lineNumber(lineNumber)
    {}

    std::wstring DXException::toString() const
    {
        _com_error error(m_errorCode);
        std::wstring errorMsg = error.ErrorMessage();

        std::wstringstream wss;
        wss << L"Exception: " << errorMsg << L" in " << m_functionName << L", " << m_filename << L", Line: " << m_lineNumber;

        return wss.str();
    }
}