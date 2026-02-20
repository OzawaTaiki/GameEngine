#pragma once

#include <Debug/ImGuiManager.h>

#include <Math/Vector/Vector4.h>

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <list>


namespace Engine {

class ImGuiDebugManager
{
public:

    static ImGuiDebugManager* GetInstance();

    void Initialize();
    void ShowDebugWindow();


    /// <summary>
    /// デバッグウィンドウを開始する
    /// </summary>
    /// <param name="_name">window名</param>
    /// <param name="_flags">ImGuiWindowFlags </param>
    /// <returns> ウィンドウが開始されたかどうか </returns>
    bool Begin(const std::string& _name = "DebugWindow");

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

    /// <summary>
    /// Colliderのデバッグウィンドウを追加する
    /// </summary>
    /// <param name="_name"></param>
    /// <param name="_func"></param>
    /// <returns></returns>
    std::string AddColliderDebugWindow(const std::string& _name, std::function<void()> _func);

    bool ChangeAllWindowVisible();
    void HideAllWindow() {isAllWindowHidden_ = true;}
    void ShowAllWindow() {isAllWindowHidden_ = false; }

    bool RegisterMenuItem(const std::string& _name, std::function<void(bool*)> _func);

private:

    void MenuBar();

    void SelectItemWindow();

    void SelectedItemWindow();

    void TabFlagsWindow();

private:
#ifdef _DEBUG
    ImGuiTabBarFlags tabBarFlags_ = ImGuiTabBarFlags_None;
#endif // _DEBUG

    std::map<std::string, std::function<void()>> debugWindows_;
    std::map<std::string, std::function<void()>> colliderDebugWindows_;
    std::vector<uint8_t> isSelect_;
    std::vector<uint8_t> colliderIsSelect_;


    bool isDemoWindowVisible_ = false;
    bool isIDStackToolVisible_ = false;
    //bool isStyleEditorVisible_ = false;

    std::map<std::string, std::function<void(bool*)>> menuItems_;

    std::map<std::string, bool> menuItemsVisibility_;
    std::map<std::string, bool> windowsVisibility_;

    // すべてのウィンドウを隠すか
    bool isAllWindowHidden_ = false;

private:
    ImGuiDebugManager();
    ~ImGuiDebugManager();

    // コピー禁止
    ImGuiDebugManager(const ImGuiDebugManager&) = delete;
    ImGuiDebugManager& operator=(const ImGuiDebugManager&) = delete;
    ImGuiDebugManager(ImGuiDebugManager&&) = delete;
    ImGuiDebugManager& operator=(ImGuiDebugManager&&) = delete;


};

} // namespace Engine
