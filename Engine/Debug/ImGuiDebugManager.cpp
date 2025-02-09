#include "ImGuiDebugManager.h"

#include <Debug/ImguITools.h>

ImGuiDebugManager* ImGuiDebugManager::GetInstance()
{
    static ImGuiDebugManager instance;
    return &instance;
}

void ImGuiDebugManager::Initialize()
{
    debugWindows_.clear();
    isSelect_.clear();
}

void ImGuiDebugManager::ShowDebugWindow()
{
#ifdef _DEBUG
    ImGui::Begin("Debug");
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

            if(isSelect[0])
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

        ImGui::Separator();

        size_t i = 0;
        for (auto& [name, func] : debugWindows_)
        {
            bool flag = isSelect_[i];
            if (ImGui::Selectable(name.c_str(), &flag))
                isSelect_[i] = flag;

            ++i;
        }
    }
    ImGui::End();

    ImGui::Begin("DebugWindow");
    {
        ImGui::BeginTabBar("DebugWindow",tabBarFlags_);
        {
            size_t i = 0;
            for (auto& [name, func] : debugWindows_)
            {
                if (isSelect_[i])
                {
                    bool flag = isSelect_[i];
                    if (ImGui::BeginTabItem(name.c_str(), reinterpret_cast<bool*>(&flag)))
                    {
                        func();
                        ImGui::EndTabItem();
                    }
                    isSelect_[i] = flag;
                }
                i++;
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();

#endif // _DEBUG
}


std::string ImGuiDebugManager::AddDebugWindow(const std::string& _name, std::function<void()> _func)
{
    std::string name = _name;
    int32_t count = 0;

    // すでに同じ名前のデバッグウィンドウが存在する場合、名前を変更する
    if (debugWindows_.contains(name))
    {
        auto it = debugWindows_.find(name);

        while (it != debugWindows_.end());
        {
            if (it->first == name)
                count++;

            it++;
        }
        name = name + std::to_string(count);
    }


    debugWindows_[name] = _func;
    isSelect_.push_back(false);

    return name;
}

void ImGuiDebugManager::RemoveDebugWindow(const std::string& _name)
{
    auto it = debugWindows_.find(_name);
    if (it != debugWindows_.end())
    {
        debugWindows_.erase(it);
    }
}
