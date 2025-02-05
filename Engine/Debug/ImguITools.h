#pragma once

#include <imgui.h>

#include <Math/Vector/Vector4.h>

#include <list>

namespace ImGuiTool
{
    void DrawTitleBar(const char* _text, const ImVec4& _color);

    void GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors);

    void DrawGradientEditor(std::list<std::pair<float, Vector4>>& _colors);
}