#include <Debug/Debug.h>
#include <Windows.h>

namespace Debug {

	void Log(const std::string& message)
	{
		OutputDebugStringA(message.c_str());
	}
}

