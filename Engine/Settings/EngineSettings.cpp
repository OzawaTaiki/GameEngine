#include "EngineSettings.h"

#include <Features/Json/Loader/JsonFileIO.h>
#include <Utility/ConvertString/ConvertString.h>
#include <Debug/Debug.h>


namespace Engine {

uint32_t EngineConfig::GetWindowWidth() const
{
#ifdef _DEBUG
    return debugWindowWidth;
#else
    return windowWidth;
#endif // _DEBUG
}

uint32_t EngineConfig::GetWindowHeight() const
{
#ifdef _DEBUG
    return debugWindowHeight;
#else
    return windowHeight;
#endif // _DEBUG
}


EngineConfig EngineSettings::default_ = {
    L"GameEngine", // windowTitle
    1280,          // renderWidth
    720,           // renderHeight
    1280,          // windowWidth
    720,           // windowHeight
    1920,          // debugWindowWidth
    1080,          // debugWindowHeight
    true,          // enableVSync
    60             // targetFPS
};

EngineConfig EngineSettings::current_ = EngineSettings::default_;

void EngineSettings::Load(const std::string& filePath)
{
    json data = JsonFileIO::Load(filePath, "");

    // 設定ファイルが無いときは，編集できるようデフォルト値で書き出しておく
    if (data.is_null())
    {
        Debug::Log("EngineSettings: config not found. creating default -> " + filePath + "\n");

        current_ = default_;
        Save(filePath);
        return;
    }

    if (data.contains("windowTitle"))   current_.windowTitle    = ConvertString(data["windowTitle"].get<std::string>());
    if (data.contains("windowWidth"))   current_.windowWidth    = data["windowWidth"].get<uint32_t>();
    if (data.contains("windowHeight"))  current_.windowHeight   = data["windowHeight"].get<uint32_t>();
    if (data.contains("enableVSync"))   current_.enableVSync    = data["enableVSync"].get<bool>();
    if (data.contains("targetFPS"))     current_.targetFPS      = data["targetFPS"].get<uint32_t>();

    // renderWidth/Height が無い古い設定ファイルは windowWidth/Height を描画解像度として扱う
    current_.renderWidth = data.contains("renderWidth")
        ? data["renderWidth"].get<uint32_t>() : current_.windowWidth;
    current_.renderHeight = data.contains("renderHeight")
        ? data["renderHeight"].get<uint32_t>() : current_.windowHeight;

    // debugWindow が無ければ通常のウィンドウサイズを使う
    current_.debugWindowWidth = data.contains("debugWindowWidth")
        ? data["debugWindowWidth"].get<uint32_t>() : current_.windowWidth;
    current_.debugWindowHeight = data.contains("debugWindowHeight")
        ? data["debugWindowHeight"].get<uint32_t>() : current_.windowHeight;

    // 0や極端な値が入っていると初期化に失敗するので丸める
    if (current_.renderWidth == 0)       current_.renderWidth = default_.renderWidth;
    if (current_.renderHeight == 0)      current_.renderHeight = default_.renderHeight;
    if (current_.windowWidth == 0)       current_.windowWidth = default_.windowWidth;
    if (current_.windowHeight == 0)      current_.windowHeight = default_.windowHeight;
    if (current_.debugWindowWidth == 0)  current_.debugWindowWidth = default_.debugWindowWidth;
    if (current_.debugWindowHeight == 0) current_.debugWindowHeight = default_.debugWindowHeight;
    if (current_.targetFPS == 0)         current_.targetFPS = default_.targetFPS;
}

void EngineSettings::Save(const std::string& filePath)
{
    json data;

    data["windowTitle"] = ConvertString(current_.windowTitle);
    data["renderWidth"] = current_.renderWidth;
    data["renderHeight"] = current_.renderHeight;
    data["windowWidth"] = current_.windowWidth;
    data["windowHeight"] = current_.windowHeight;
    data["debugWindowWidth"] = current_.debugWindowWidth;
    data["debugWindowHeight"] = current_.debugWindowHeight;
    data["enableVSync"] = current_.enableVSync;
    data["targetFPS"] = current_.targetFPS;

    JsonFileIO::Save(filePath, "", data);
}

} // namespace Engine
