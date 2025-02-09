#pragma once

#include <Debug/ImGuiManager.h>

#include <Math/Vector/Vector4.h>

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <list>

class ImGuiDebugManager
{
public:

    static ImGuiDebugManager* GetInstance();

    void Initialize();
    void ShowDebugWindow();

    /// <summary>
    /// デバッグウィンドウを追加する
    /// </summary>
    /// <param name="_name">識別用の名前 </param>
    /// <param name="_func">Imgui関数 </param>
    /// <returns>識別用の名前 </returns>
    std::string AddDebugWindow(const std::string& _name, std::function<void()> _func);

    /// <summary>
    /// デバッグウィンドウを削除する
    /// </summary>
    /// <param name="_name">識別用の名前 </param>
    void RemoveDebugWindow(const std::string& _name);



private:
#ifdef _DEBUG
    ImGuiTabBarFlags tabBarFlags_ = ImGuiTabBarFlags_None;
#endif // _DEBUG

    std::map<std::string, std::function<void()>> debugWindows_;
    std::vector<uint8_t> isSelect_;


    ImGuiDebugManager() = default;
    ~ImGuiDebugManager() = default;

    // コピー禁止
    ImGuiDebugManager(const ImGuiDebugManager&) = delete;
    ImGuiDebugManager& operator=(const ImGuiDebugManager&) = delete;
    ImGuiDebugManager(ImGuiDebugManager&&) = delete;
    ImGuiDebugManager& operator=(ImGuiDebugManager&&) = delete;


};