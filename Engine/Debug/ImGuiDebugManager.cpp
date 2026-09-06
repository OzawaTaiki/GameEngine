#include "ImGuiDebugManager.h"

#include <System/input/Input.h>
#include <Debug/ImguITools.h>
#include <Debug/GameViewportWindow.h>
#include <Debug/DebugConsole.h>
#include <Debug/DebugStats.h>
#include <Debug/EditorLayout.h>
#include <Debug/EditorWindowManager.h>

#include <algorithm>
#include <set>


namespace Engine {

ImGuiDebugManager* ImGuiDebugManager::GetInstance()
{
    static ImGuiDebugManager instance;
    return &instance;
}

ImGuiDebugManager::ImGuiDebugManager()
{
}

ImGuiDebugManager::~ImGuiDebugManager()
{
    debugWindows_.clear();
    colliderDebugWindows_.clear();
    isSelect_.clear();

    windowsVisibility_.clear();

    for (auto& [name, func] : menuItems_)
    {
        if (func)
            func = nullptr;
    }
    menuItems_.clear();
}

void ImGuiDebugManager::Initialize()
{
    debugWindows_.clear();
    colliderDebugWindows_.clear();
    isSelect_.clear();
    colliderIsSelect_.clear();

    windowsVisibility_.clear();
    menuItems_.clear();

    isAllWindowHidden_ = false;

    // エディタとして常用する枠はデフォルトで開いておく
    windowsVisibility_["Hierarchy"] = true;
    windowsVisibility_["Inspector"] = true;
    isConsoleVisible_ = true;
    isStatsVisible_ = true;
}

void ImGuiDebugManager::ShowDebugWindow()
{
#ifdef _DEBUG

    // 非表示中も計測は続ける
    DebugStats::GetInstance()->Update();

    if (Input::GetInstance()->IsKeyTriggered(DIK_F3))
    {
        isAllWindowHidden_ = !isAllWindowHidden_;
    }

    if(isAllWindowHidden_)
        return;

    MenuBar();


    for (auto& [name, func] : menuItems_)
    {
        if (menuItemsVisibility_[name] && func)
        {
            func(&menuItemsVisibility_[name]);
        }
    }

    SelectedItemWindow();
    SelectItemWindow();
    TabFlagsWindow();

    if (isConsoleVisible_)
        DebugConsole::GetInstance()->Show(&isConsoleVisible_);

    if (isStatsVisible_)
        DebugStats::GetInstance()->Show(&isStatsVisible_);

    // 自前で登録されたウィンドウ
    EditorWindowManager::GetInstance()->Draw();

#endif // _DEBUG
}

bool ImGuiDebugManager::Begin([[maybe_unused]] const std::string& _name)
{
#ifdef _DEBUG
    // 始めてのとき
    // この名前が含まれていないとき
    if (!windowsVisibility_.contains(_name))
    {
        // 新しいウィンドウの場合は、デフォルトで非表示にする
        windowsVisibility_[_name] = false;
    }

    if (!windowsVisibility_[_name] || isAllWindowHidden_)
    {
        return false; // ウィンドウが非表示の場合は何もしない
    }

    ImGui::Begin(_name.c_str(), &windowsVisibility_[_name]);

#endif // _DEBUG
    return true;
}


std::string ImGuiDebugManager::AddDebugWindow(const std::string& _name, std::function<void()> _func)
{
    std::string name = _name;
    int32_t count = 0;

    // すでに同じ名前のデバッグウィンドウが存在する場合、名前を変更する
    if (debugWindows_.contains(name))
    {
        auto it = debugWindows_.find(name);

        while (debugWindows_.find(name) != debugWindows_.end())
        {
            std::string num;
            if(count < 10)
                num = "0";
            num = std::to_string(count);
            name = _name + num;
            it++;
            ++count;
        }
    }


    debugWindows_[name] = _func;
    isSelect_[name] = false;

    return name;
}

void ImGuiDebugManager::RemoveDebugWindow(const std::string& _name)
{
    auto it = debugWindows_.find(_name);
    if (it != debugWindows_.end())
    {
        debugWindows_.erase(it);
        isSelect_.erase(_name);
        return;
    }

    it = colliderDebugWindows_.find(_name);
    if (it != colliderDebugWindows_.end())
    {
        colliderDebugWindows_.erase(it);
        colliderIsSelect_.erase(_name);
        return;
    }

    auto it2 = menuItems_.find(_name);
    if (it2 != menuItems_.end())
    {
        menuItems_.erase(it2);
        return;
    }
}

std::string ImGuiDebugManager::AddColliderDebugWindow(const std::string& _name, std::function<void()> _func)
{

    std::string name = _name;
    int32_t count = 0;

    // すでに同じ名前のデバッグウィンドウが存在する場合、名前を変更する
    if (colliderDebugWindows_.contains(name))
    {
        auto it = colliderDebugWindows_.find(name);

        while (colliderDebugWindows_.find(name) != colliderDebugWindows_.end())
        {
            std::string num;
            if (count < 10)
                num = "0";
            num += std::to_string(count);
            name = _name + num;
            it++;
            ++count;
        }
    }


    colliderDebugWindows_[name] = _func;
    colliderIsSelect_[name] = false;

    return name;
}

bool ImGuiDebugManager::ChangeAllWindowVisible()
{
    isAllWindowHidden_ = !isAllWindowHidden_;
    return isAllWindowHidden_;
}

bool ImGuiDebugManager::RegisterMenuItem(const std::string& _name, std::function<void(bool*)> _func)
{
    if (_func == nullptr)
        return false;

    menuItems_[_name] = _func;
    menuItemsVisibility_[_name] = false; // デフォルトで非表示する

    return false;
}

void ImGuiDebugManager::MenuBar()
{
#ifdef _DEBUG
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("ImGui Tools"))
        {
            ImGui::MenuItem("Demo Window", nullptr, &isDemoWindowVisible_);
            ImGui::MenuItem("IDStack", nullptr, &isIDStackToolVisible_);
            ImGui::EndMenu();
        }
        ImGui::Separator();  // 区切り線
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Game", "F2", GameViewportWindow::GetInstance()->GetEnablePtr());
            ImGui::MenuItem("Hierarchy", nullptr, &windowsVisibility_["Hierarchy"]);
            ImGui::MenuItem("Inspector", nullptr, &windowsVisibility_["Inspector"]);
            ImGui::MenuItem("Console", nullptr, &isConsoleVisible_);
            ImGui::MenuItem("Stats", nullptr, &isStatsVisible_);

            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout"))
            {
                // 閉じたままだと配置し直しても見えないので開く
                windowsVisibility_["Hierarchy"] = true;
                windowsVisibility_["Inspector"] = true;
                isConsoleVisible_ = true;
                isStatsVisible_ = true;
                GameViewportWindow::GetInstance()->SetEnable(true);

                EditorLayout::GetInstance()->RequestReset();
            }

            ImGui::EndMenu();
        }
        ImGui::Separator();  // 区切り線
        if (ImGui::BeginMenu("Core"))
        {
            ImGui::MenuItem("Time Info", nullptr, &menuItemsVisibility_["TimeInfo"]);
            ImGui::MenuItem("Scene Manager", nullptr, &menuItemsVisibility_["SceneManager"]);
            ImGui::MenuItem("Input Status", nullptr, &menuItemsVisibility_["InputStatus"]);
            ImGui::EndMenu();
        }
        ImGui::Separator();  // 区切り線
        if (ImGui::BeginMenu("Resource"))
        {
            ImGui::MenuItem("Texture Manager", nullptr, &menuItemsVisibility_["TextureManager"]);
            ImGui::MenuItem("Model Manager", nullptr, &menuItemsVisibility_["ModelManager"]);
            ImGui::EndMenu();
        }
        ImGui::Separator();  // 区切り線
        if (ImGui::BeginMenu("System"))
        {
            ImGui::MenuItem("Collision Manager", nullptr, &menuItemsVisibility_["CollisionManager"]);
            ImGui::EndMenu();
        }
        ImGui::Separator();  // 区切り線

        ImGui::Separator();  // 区切り線
        if (ImGui::BeginMenu("Custom"))
        {
            // EditorWindowManager に登録されたもの
            EditorWindowManager::GetInstance()->DrawMenu();

            // RegisterMenuItem で登録されたもののうち，上のメニューに直書きしていないもの
            // これが無いとエンジン外から登録した項目に手が届かない
            static const std::set<std::string> kBuiltInMenuItems =
            {
                "TimeInfo", "SceneManager", "InputStatus",
                "TextureManager", "ModelManager", "CollisionManager",
            };

            bool hasCustomMenuItem = false;
            for (auto& [name, func] : menuItems_)
            {
                if (kBuiltInMenuItems.contains(name))
                    continue;

                if (!hasCustomMenuItem)
                {
                    ImGui::Separator();
                    hasCustomMenuItem = true;
                }

                ImGui::MenuItem(name.c_str(), nullptr, &menuItemsVisibility_[name]);
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();  // 区切り線

        // ->window Visivility
        // ->をクリックでポップアップとかでウィンドウ表示
        // ポップアップ内でthis->Beginのウィンドウたちの表示切り替えをおこなえるようにする チェックボックス
        if (ImGui::BeginMenu("Windows"))
        {
            for (auto& [name, isVisible] : windowsVisibility_)
            {
                ImGui::MenuItem(name.c_str(), nullptr, &isVisible);
                windowsVisibility_[name] = isVisible;
            }
            ImGui::EndMenu();
        }

        float rightItemWidth = 170.0f; // 右端要素の幅Z

        // 右端に移動
        ImGui::SameLine(ImGui::GetWindowWidth() - rightItemWidth);
        ImGui::Separator();  // 区切り線
        ImGui::Checkbox("Hide Windows", &isAllWindowHidden_);


        ImGui::EndMainMenuBar();
    }

    if (isDemoWindowVisible_)
        ImGui::ShowDemoWindow(&isDemoWindowVisible_);
    if (isIDStackToolVisible_)
        ImGui::ShowIDStackToolWindow(&isIDStackToolVisible_);

#endif // _DEBUG
}

void ImGuiDebugManager::SelectItemWindow()
{
    // 登録されているデバッグウィンドウの一覧
#ifdef _DEBUG
    if (this->Begin("Hierarchy"))
    {
        hierarchyFilter_.Draw("##HierarchyFilter", -1.0f);

        if (ImGui::SmallButton("All"))
        {
            for (auto& [name, isSelect] : isSelect_)
                isSelect = true;
            for (auto& [name, isSelect] : colliderIsSelect_)
                isSelect = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("None"))
        {
            for (auto& [name, isSelect] : isSelect_)
                isSelect = false;
            for (auto& [name, isSelect] : colliderIsSelect_)
                isSelect = false;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%zu items", debugWindows_.size() + colliderDebugWindows_.size());

        ImGui::Separator();

        if (ImGui::BeginChild("HierarchyList"))
        {
            ImGui::SeparatorText("Objects");
            for (auto& [name, func] : debugWindows_)
            {
                if (!hierarchyFilter_.PassFilter(name.c_str()))
                    continue;

                bool flag = isSelect_[name];
                if (ImGui::Selectable((name + "##Debug").c_str(), &flag))
                    isSelect_[name] = flag;
            }

            ImGui::SeparatorText("Colliders");
            for (auto& [name, func] : colliderDebugWindows_)
            {
                if (name == "CollisionManager") continue;

                if (!hierarchyFilter_.PassFilter(name.c_str()))
                    continue;

                bool flag = colliderIsSelect_[name];
                if (ImGui::Selectable((name + "##ColliderDebug").c_str(), &flag))
                    colliderIsSelect_[name] = flag;
            }
        }
        ImGui::EndChild();

        ImGui::End();
    }
#endif // _DEBUG
}

void ImGuiDebugManager::SelectedItemWindow()
{
#ifdef _DEBUG

    static bool openDebugWindow = true;
    static bool openColliderDebugWindow = true;

    if (this->Begin("Inspector"))
    {
        ImGui::Checkbox("DebugWindow", &openDebugWindow);
        ImGui::SameLine();
        ImGui::Checkbox("ColliderWindow", &openColliderDebugWindow);

        // 何も選ばれていないと空欄になって迷うので案内を出す
        const bool hasSelection =
            std::any_of(isSelect_.begin(), isSelect_.end(), [](const auto& _p) { return _p.second; }) ||
            std::any_of(colliderIsSelect_.begin(), colliderIsSelect_.end(), [](const auto& _p) { return _p.second; });

        if (!hasSelection)
            ImGui::TextDisabled("Select an item in the Hierarchy.");

        if (openDebugWindow)
        {

            ImGui::BeginTabBar("DebugWindow", tabBarFlags_);
            {
                for (auto& [name, func] : debugWindows_)
                {
                    std::string label = name + "##Debug";

                    if (isSelect_[name])
                    {
                        bool flag = isSelect_[name];
                        if (ImGui::BeginTabItem(label.c_str(), &flag))
                        {
                            func();
                            ImGui::EndTabItem();
                        }
                        isSelect_[name] = flag;
                    }
                }
            }
            ImGui::EndTabBar();
        }

        if (openColliderDebugWindow)
        {
            ImGui::SeparatorText("Colliders");
            ImGui::BeginTabBar("ColliderDebugWindow", tabBarFlags_);
            {
                for (auto& [name, func] : colliderDebugWindows_)
                {
                    if (name == "CollisionManager") continue;

                    if (colliderIsSelect_[name])
                    {
                        std::string label = name + "##ColliderDebug";

                        bool flag = colliderIsSelect_[name];
                        if (ImGui::BeginTabItem(label.c_str(), &flag))
                        {
                            func();
                            ImGui::EndTabItem();
                        }
                        colliderIsSelect_[name] = flag;
                    }
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
#endif // _DEBUG
}

void ImGuiDebugManager::TabFlagsWindow()
{
#ifdef _DEBUG
    if(this->Begin("Tab Flags"))
    {
        static bool isSelect[9] = { true };
        if (ImGui::CollapsingHeader("tabBarFlags"))
        {
            ImGui::Checkbox("ImGuiTabBarFlags_None", &isSelect[0]);

            ImGui::BeginDisabled(isSelect[0]);
            {
                ImGui::Checkbox("ImGuiTabBarFlags_Reorderable", &isSelect[1]);
                ImGui::Checkbox("ImGuiTabBarFlags_AutoSelectNewTabs", &isSelect[2]);
                ImGui::Checkbox("ImGuiTabBarFlags_TabListPopupButton", &isSelect[3]);
                ImGui::Checkbox("ImGuiTabBarFlags_NoCloseWithMiddleMouseButton", &isSelect[4]);
                ImGui::Checkbox("ImGuiTabBarFlags_NoTabListScrollingButtons", &isSelect[5]);
                ImGui::Checkbox("ImGuiTabBarFlags_NoTooltip", &isSelect[6]);
                ImGui::Checkbox("ImGuiTabBarFlags_FittingPolicyResizeDown", &isSelect[7]);
                ImGui::Checkbox("ImGuiTabBarFlags_FittingPolicyScroll", &isSelect[8]);
            }
            ImGui::EndDisabled();

            if (isSelect[0])
                tabBarFlags_ = ImGuiTabBarFlags_None;

            else
            {
                if (isSelect[1])
                    tabBarFlags_ |= ImGuiTabBarFlags_Reorderable;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_Reorderable;
                if (isSelect[2])
                    tabBarFlags_ |= ImGuiTabBarFlags_AutoSelectNewTabs;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_AutoSelectNewTabs;
                if (isSelect[3])
                    tabBarFlags_ |= ImGuiTabBarFlags_TabListPopupButton;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_TabListPopupButton;
                if (isSelect[4])
                    tabBarFlags_ |= ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_NoCloseWithMiddleMouseButton;
                if (isSelect[5])
                    tabBarFlags_ |= ImGuiTabBarFlags_NoTabListScrollingButtons;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_NoTabListScrollingButtons;
                if (isSelect[6])
                    tabBarFlags_ |= ImGuiTabBarFlags_NoTooltip;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_NoTooltip;
                if (isSelect[7])
                    tabBarFlags_ |= ImGuiTabBarFlags_FittingPolicyResizeDown;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_FittingPolicyResizeDown;
                if (isSelect[8])
                    tabBarFlags_ |= ImGuiTabBarFlags_FittingPolicyScroll;
                else
                    tabBarFlags_ &= ~ImGuiTabBarFlags_FittingPolicyScroll;
            }

        }

        ImGui::End();
    }
#endif // _DEBUG
}

} // namespace Engine
