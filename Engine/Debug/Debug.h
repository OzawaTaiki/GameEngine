#pragma once
#include <format>
#include <string>
#include <vector>


namespace Engine {

namespace Debug {
	void Log(const std::string& message);

    void Log(const std::wstring& message);

    void LogWarning(const std::string& message);

    void LogError(const std::string& message);

};

// HRESULTのエラーチェックとログ出力
#define CHECK_HR(hr, msg) \
    if (FAILED(hr)) { \
        Debug::LogError(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
                   __FUNCTION__ + ": " + msg + " HRESULT=0x" + \
                   std::to_string(static_cast<unsigned long>(hr))); \
        return nullptr; \
    }
// HRESULTのエラーチェックとログ出力（戻り値がない場合）
#define CHECK_HR_VOID(hr, msg) \
    if (FAILED(hr)) { \
        Debug::LogError(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
                   __FUNCTION__ + ": " + msg + " HRESULT=0x" + \
                   std::to_string(static_cast<unsigned long>(hr))); \
        return; \
    }

// ログ出力マクロ
#define LOG_ERROR(msg) \
    Debug::LogError(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
               __FUNCTION__ + ": " + msg)

#define LOG_WARNING(msg) \
    Debug::LogWarning(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
               __FUNCTION__ + ": " + msg)

} // namespace Engine
