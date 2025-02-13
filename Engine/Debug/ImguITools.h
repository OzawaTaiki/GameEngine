#pragma once

#include <imgui.h>

#include <Math/Vector/Vector4.h>

#include <list>
#include <DirectXTex.h>

class AnimationSequence;

namespace ImGuiTool
{

    void GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors);

    void TimeLine(const char* _label, AnimationSequence* _sequence);
}