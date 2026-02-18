#pragma once

#include <imgui.h>

#include <Math/Vector/Vector4.h>

#include <list>
#include <DirectXTex.h>

#include <Features/Animation/Sequence/SequenceEvent.h>

#ifdef _DEBUG

class AnimationSequence;

namespace ImGuiTool
{
    namespace TimeLineConstants
    {
        constexpr float kTrackHeight = 20.0f;          // トラックの高さ
        constexpr float kTrackHeaderWidth = 150.0f;    // トラックヘッダーの幅
        constexpr float kTimeScale = 100.0f;           // 1秒あたりのピクセル数
        constexpr float kTimeMarkerSize = 7.0f;        // タイムマーカーの大きさ
        constexpr float kCurrentTimeMarkerSize = 11.0f; // 現在時刻マーカーのサイズ
        constexpr float kTimelineHeightPadding = 5.0f;  // タイムラインの上下のパディング
        constexpr float kSnapThreshold = 0.05f;        // スナップするしきい値（秒）
        constexpr float kControlPanelHeight = 60.0f;   // コントロールパネルの高さ
        constexpr float kTimeScaleHeaderHeight = 20.0f; // 時間目盛りヘッダーの高さ
        constexpr float kStatusBarHeight = 20.0f;      // ステータスバーの高さ
        constexpr float kValueDisplayWidth = 150.0f;   // 値表示エリアの幅

        // タイムライン内の色定義
        const ImU32 kTimelineBgColor = IM_COL32(40, 40, 40, 255);
        const ImU32 kTimelineGridColor = IM_COL32(60, 60, 60, 255);
        const ImU32 kTimelineGridMajorColor = IM_COL32(80, 80, 80, 255);
        const ImU32 kTrackBgColor = IM_COL32(50, 50, 50, 180);
        const ImU32 kTrackBgColorSelected = IM_COL32(70, 70, 80, 200);
        const ImU32 kCurrentTimeMarkerColor = IM_COL32(255, 120, 0, 255);
        const ImU32 kKeyframeColor = IM_COL32(255, 255, 255, 255);
        const ImU32 kKeyframeColorSelected = IM_COL32(255, 80, 80, 255);
        const ImU32 kKeyframeColorHover = IM_COL32(255, 180, 100, 255);
    }

    struct TimelineState
    {
        float currentTime = 0.0f;
        float maxTime = 10.0f;
        float viewStart = 0.0f;
        float viewEnd = 10.0f;
        float timeScale = TimeLineConstants::kTimeScale;
        bool showSnapping = true;
        float snappingInterval = 0.1f;
        bool isPlaying = false;
        bool isDragging = false;
        SequenceEvent::KeyFrame* draggingKeyFrame = nullptr;
        float trackIndex = 0.0f;
    };

    void GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors);

    void TimeLine(const char* _label, AnimationSequence* _sequence);
}

#endif // _DEBUG