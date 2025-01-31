#include "ImGuiHelper.h"

#include <cmath>

void ImGuiHelper::DrawRightPentagon(const ImVec2& _center, const ImVec2& _inscribedRect, float _angle, ImU32 _color)
{

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const int kNumPoints = 5;
    ImVec2 points[kNumPoints];

    // 下基準の座標
    // 下
    points[0] = ImVec2(_inscribedRect.x / 2.0f, _inscribedRect.y); // 四角形の下中央
    // 左
    points[1] = ImVec2(0, _inscribedRect.y / 2.0f); // 四角形の左中央
    // 左上
    points[2] = ImVec2(0, 0); // 四角形の左上
    // 右上
    points[3] = ImVec2(_inscribedRect.x, 0); // 四角形の右上
    // 右
    points[4] = ImVec2(_inscribedRect.x, _inscribedRect.y / 2.0f); // 四角形の右中央


    // 回転 と 中心座標の移動
    for (auto& point : points)
    {
        float x = point.x - _inscribedRect.x / 2.0f;
        float y = point.y - _inscribedRect.y / 2.0f;

        point.x = x * cosf(_angle) - y * sinf(_angle);
        point.y = x * sinf(_angle) + y * cosf(_angle);

        point.x += _center.x;
        point.y += _center.y;
    }

    drawList->AddConvexPolyFilled(points, kNumPoints, _color);

}

