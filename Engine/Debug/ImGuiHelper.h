#pragma once

#include <imgui.h>

#include <Math/Vector/Vector4.h>

#include <list>

class AnimationSequence;
namespace ImGuiHelper
{
    /// <summary>
    /// 直角五角形を描画する
    /// </summary>
    /// <param name="_label">ラベル</param>
    /// <param name="_center">ウィンドウのローカル座標</param>
    /// <param name="_inscribedRect">内接する四角形</param>
    /// <param name="_angle">方向</param>
    /// <param name="_color">色</param>
    /// <param name="_hoverColor">ホバー時の色</param>
    /// <returns>クリック</returns>
    void DrawRightPentagon(const ImVec2& _center, const ImVec2& _inscribedRect, float _angle = 0.0f, ImU32 _color = ImU32(0xffffffff));

    /// <summary>
    /// グラデーションエディタを描画する
    /// </summary>
    /// <param name="_colors"> 変更する色のリスト </param>
    void DrawGradientEditor(std::list<std::pair<float, Vector4>>& _colors);

    namespace TimeLine
    {

        /// <summary>
        /// タイムラインを描画する
        /// </summary>
        /// <param name="_sequence"></param>
        void Draw(AnimationSequence* _sequence);

        void CreateEventButton(AnimationSequence* _sequence);
    }


    /// <summary>
    /// タイトルバーを描画する
    /// </summary>
    /// <param name="_text">タイトル</param>
    /// <param name="_color">背景の色</param>
    void DrawTitleBar(const char* _text, const ImVec4& _color);


}