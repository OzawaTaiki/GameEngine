#pragma once
#include <format>
#include <string>
#include <vector>


namespace Engine {

namespace Debug {
	void Log(const std::string& message);

    void Log(const std::wstring& message);

};

// HRESULTのエラーチェックとログ出力
#define CHECK_HR(hr, msg) \
    if (FAILED(hr)) { \
        Debug::Log(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
                   __FUNCTION__ + ": " + msg + " HRESULT=0x" + \
                   std::to_string(static_cast<unsigned long>(hr))); \
        return nullptr; \
    }
// HRESULTのエラーチェックとログ出力（戻り値がない場合）
#define CHECK_HR_VOID(hr, msg) \
    if (FAILED(hr)) { \
        Debug::Log(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
                   __FUNCTION__ + ": " + msg + " HRESULT=0x" + \
                   std::to_string(static_cast<unsigned long>(hr))); \
        return; \
    }

// ログ出力マクロ
#define LOG_ERROR(msg) \
    Debug::Log(std::string(__FILE__) + "(" + std::to_string(__LINE__) + ") " + \
               __FUNCTION__ + ": " + msg)

} // namespace Engine
