#pragma once

#include <string>

namespace StringUtils
{
    /// <summary>
    /// 指定文字移行の文字列を取得
    /// </summary>
    /// <param name="_str">操作を行う文字列</param>
    /// <param name="_delimiter">対象の文字</param>
    /// <returns>操作後の文字列</returns>
    std::string GetAfter(const std::string& _str, char _delimiter);

    /// <summary>
    /// 指定文字列移行の文字列を取得
    /// </summary>
    /// <param name="_str">操作を行う文字列</param>
    /// <param name="_delimiter">対象の文字列</param>
    /// <returns>操作後の文字列</returns>
    std::string GetAfter(const std::string& _str, const std::string& _delimiter);

    /// <summary>
    /// 指定文字以前の文字列を取得
    /// </summary>
    /// <param name="_str">操作を行う文字列</param>
    /// <param name="_delimiter">対象の文字</param>
    /// <returns>操作後の文字列</returns>
    std::string GetBefore(const std::string& _str, char _delimiter);


    /// <summary>
    /// 指定文字以前の文字列を取得
    /// </summary>
    /// <param name="_str">操作を行う文字列</param>
    /// <param name="_delimiter">対象の文字列</param>
    /// <returns>操作後の文字列</returns>
    std::string GetBefore(const std::string& _str, const std::string& _delimiter);

    /// <summary>
    /// 文字列の拡張子を取得
    /// </summary>
    /// <param name="_str">操作を行う文字列</param>
    /// <returns>拡張子</returns>
    std::string GetExtension(const std::string& _str);

} // namespace StringUtils