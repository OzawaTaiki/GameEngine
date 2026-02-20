#include <Debug/Debug.h>
#include <Windows.h>


namespace Engine {

namespace Debug {

	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());
	}
	void Log(const std::wstring& message)
	{
        OutputDebugStringW(message.c_str());
	}
}

} // namespace Engine
