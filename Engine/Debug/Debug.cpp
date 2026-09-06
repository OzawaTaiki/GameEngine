#include <Debug/Debug.h>
#include <Windows.h>

#ifdef _DEBUG
#include <Debug/DebugConsole.h>
#endif // _DEBUG


namespace Engine {

namespace Debug {

	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());

#ifdef _DEBUG
		DebugConsole::GetInstance()->AddLog(LogLevel::Log, message);
#endif // _DEBUG
	}
	void Log(const std::wstring& message)
	{
        OutputDebugStringW(message.c_str());

#ifdef _DEBUG
		// コンソールは UTF-8 で保持するので変換する
		if (!message.empty())
		{
			const int size = WideCharToMultiByte(CP_UTF8, 0, message.c_str(), static_cast<int>(message.size()),
												 nullptr, 0, nullptr, nullptr);
			std::string converted(static_cast<size_t>(size), '\0');
			WideCharToMultiByte(CP_UTF8, 0, message.c_str(), static_cast<int>(message.size()),
								converted.data(), size, nullptr, nullptr);

			DebugConsole::GetInstance()->AddLog(LogLevel::Log, converted);
		}
#endif // _DEBUG
	}

	void LogWarning(const std::string& message)
	{
		OutputDebugStringA(message.c_str());

#ifdef _DEBUG
		DebugConsole::GetInstance()->AddLog(LogLevel::Warning, message);
#endif // _DEBUG
	}

	void LogError(const std::string& message)
	{
		OutputDebugStringA(message.c_str());

#ifdef _DEBUG
		DebugConsole::GetInstance()->AddLog(LogLevel::Error, message);
#endif // _DEBUG
	}
}

} // namespace Engine
