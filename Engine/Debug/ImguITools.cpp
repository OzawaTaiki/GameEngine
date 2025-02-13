#include "ImguITools.h"

#include <Debug/ImGuiHelper.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <numbers>
#include <list>



void ImGuiTool::GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors)
{

    ImGui::PushID(_label);

    ImGui::Text(_label);
    ImGui::SameLine();

    if (ImGui::Button("Open Editor"))
    {
        ImGui::OpenPopup("GradientEditor");

        // colorsをtimeでソート
        _colors.sort([](const std::pair<float, Vector4>& a, const std::pair<float, Vector4>& b) {return a.first < b.first; });

    }
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    if (ImGui::BeginPopup("GradientEditor"))
    {
        ImGuiHelper::DrawTitleBar("GradientEditor", ImVec4(1, 1, 1, 1));

        ImGuiHelper::DrawGradientEditor(_colors);

        ImGui::EndPopup();
    }


    ImGui::PopID();

}

void ImGuiTool::TimeLine(const char* _label, AnimationSequence* _sequence)
{
    // Nullチェック
    if (!_sequence)
    {
        return;
    }

    ImGui::PushID(_label);

    ImGuiHelper::TimeLine::Draw(_sequence);

    ImGui::PopID();



}

