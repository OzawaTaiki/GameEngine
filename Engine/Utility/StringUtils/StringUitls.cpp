#include "StringUitls.h"

std::string StringUtils::GetAfter(const std::string& _str, char _delimiter)
{
    size_t pos = _str.find(_delimiter);
    if (pos != std::string::npos && pos + 1 < _str.size())
    {
        return _str.substr(pos + 1);
    }
    return  ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetAfter(const std::string& _str, const std::string& _delimiter)
{
    size_t pos = _str.find(_delimiter);
    if (pos != std::string::npos && pos + _delimiter.size() < _str.size())
    {
        return _str.substr(pos + _delimiter.size());
    }
    return  ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetAfterLast(const std::string& _str, char _delimiter)
{
    size_t pos = _str.find_last_of(_delimiter);
    if (pos != std::string::npos && pos + 1 < _str.size())
    {
        return _str.substr(pos + 1);
    }
    return ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetAfterLast(const std::string& _str, const std::string& _delimiter)
{
    size_t pos = _str.rfind(_delimiter);
    if (pos != std::string::npos && pos + _delimiter.size() < _str.size())
    {
        return _str.substr(pos + _delimiter.size());
    }
    return ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetBefore(const std::string& _str, char _delimiter)
{
    size_t pos = _str.find(_delimiter);
    if (pos != std::string::npos)
    {
        return _str.substr(0, pos);
    }
    return  ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetBefore(const std::string& _str, const std::string& _delimiter)
{
    size_t pos = _str.find(_delimiter);
    if (pos != std::string::npos)
    {
        return _str.substr(0, pos);
    }
    return  ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetBeforeLast(const std::string& _str, char _delimiter)
{
    size_t pos = _str.find_last_of(_delimiter);
    if (pos != std::string::npos)
    {
        return _str.substr(0, pos);
    }
    return ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetBeforeLast(const std::string& _str, const std::string& _delimiter)
{
    size_t pos = _str.rfind(_delimiter);
    if (pos != std::string::npos)
    {
        return _str.substr(0, pos);
    }
    return ""; // デリミタが見つからない場合は空文字列を返す
}

std::string StringUtils::GetExtension(const std::string& _str)
{
    size_t pos = _str.find_last_of('.');
    if (pos != std::string::npos && pos + 1 < _str.size())
    {
        return _str.substr(pos + 1);
    }
    return ""; // 拡張子が見つからない場合は空文字列を返す
}

bool StringUtils::Contains(const std::string& _str, const std::string& _subStr)
{
    return _str.find(_subStr) != std::string::npos; // _subStrが_strに含まれているかどうかを確認
}

