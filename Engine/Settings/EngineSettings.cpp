#include "EngineSettings.h"

#include <Features/Json/Loader/JsonFileIO.h>
#include <Utility/ConvertString/ConvertString.h>

EngineConfig EngineSettings::default_ = {
    L"GameEngine", // windowTitle
    1280,          // windowWidth
    720,           // windowHeight
    false,         // isFullscreen
    true,          // enableVSync
    60             // targetFPS
};

EngineConfig EngineSettings::current_ = EngineSettings::default_;

void EngineSettings::Load(const std::string& filePath)
{
    json data = JsonFileIO::Load(filePath, "");
    if (data.is_null())
    {
        current_ = default_;
        return;
    }

    if (data.contains("windowTitle"))   current_.windowTitle    = ConvertString(data["windowTitle"].get<std::string>());
    if (data.contains("windowWidth"))   current_.windowWidth    = data["windowWidth"].get<uint32_t>();
    if (data.contains("windowHeight"))  current_.windowHeight   = data["windowHeight"].get<uint32_t>();
    if (data.contains("isFullscreen"))  current_.isFullscreen   = data["isFullscreen"].get<bool>();
    if (data.contains("enableVSync"))   current_.enableVSync    = data["enableVSync"].get<bool>();
    if (data.contains("targetFPS"))     current_.targetFPS      = data["targetFPS"].get<uint32_t>();
}

void EngineSettings::Save(const std::string& filePath)
{
    json data;

    data["windowTitle"] = ConvertString(current_.windowTitle);
    data["windowWidth"] = current_.windowWidth;
    data["windowHeight"] = current_.windowHeight;
    data["isFullscreen"] = current_.isFullscreen;
    data["enableVSync"] = current_.enableVSync;
    data["targetFPS"] = current_.targetFPS;

    JsonFileIO::Save(filePath, "", data);
}
