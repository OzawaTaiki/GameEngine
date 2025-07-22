#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>

#include <Debug/ImGuiManager.h>

struct TextParam
{
    Vector2 scale = { 1.0f, 1.0f }; // スケール
    float rotate = 0.0f; // 回転角度
    Vector2 position; // 描画位置

    bool useGradient = false; // グラデーションを使用するかどうか
    Vector4 topColor = { 1, 1, 1, 1 }; // 上の色(グラデーション用)
    Vector4 bottomColor = { 0, 0, 0, 1 }; // 下の色(グラデーション用)

    Vector2 pivot = { 0.5f, 0.5f }; // ピボット位置

    bool  useOutline = false; // アウトラインを使用するかどうか
    Vector4 outlineColor = { 0, 0, 0, 1 }; // アウトラインの色
    float outlineScale = 0.03f; // アウトラインの太さ

    TextParam& SetScale(const Vector2& _scale) { scale = _scale; return *this; }
    TextParam& SetRotate(float _rotate) { rotate = _rotate; return *this; }
    TextParam& SetPosition(const Vector2& _pos) { position = _pos; return *this; }
    TextParam& SetColor(const Vector4& _color) { topColor = _color; bottomColor = _color; useGradient = false; return *this; }
    TextParam& SetGradientColor(const Vector4& _topColor, const Vector4& _bottomColor) { topColor = _topColor; bottomColor = _bottomColor; useGradient = true; return *this; }
    TextParam& SetPivot(const Vector2& _piv) { pivot = _piv; return *this; }
    TextParam& SetOutline(const Vector4& _outlineColor = { 0, 0, 0, 1 }, float _outlineScale = 0.03f)
    {
        useOutline = true;
        outlineColor = _outlineColor;
        outlineScale = _outlineScale;
        return *this;
    }

    void ImGui()
    {
        ImGui::Checkbox("Outline", &useOutline);
        ImGui::Checkbox("Gradient", &useGradient);

        ImGui::DragFloat2("Scale", &scale.x, 0.01f);
        ImGui::DragFloat("Rotate", &rotate, 0.01f);
        ImGui::DragFloat2("Pivot", &pivot.x, 0.01f);
        ImGui::Checkbox("Use Gradient", &useGradient);
        ImGui::Checkbox("Use Outline", &useOutline);
        ImGui::ColorEdit4("Bottom Color", &bottomColor.x);
        ImGui::ColorEdit4("Top Color", &topColor.x);
        ImGui::ColorEdit4("Outline Color", &outlineColor.x);
        ImGui::DragFloat("Outline Scale", &outlineScale, 0.01f);
    }
};
