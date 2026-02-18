#include "ImguITools.h"

#include <Debug/ImGuiHelper.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>


#include <Features/Animation/Sequence/AnimationSequence.h>
#include <Math/Easing.h>

#include <numbers>
#include <list>
#include <algorithm>

#ifdef _DEBUG


void ImGuiTool::GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors)
{

    ImGui::PushID(_label);

    ImGui::Text(_label);
    ImGui::SameLine();

    if (ImGui::Button("Open Editor"))
    {
        ImGui::OpenPopup("GradientEditor");

        // colorsをtimeでソート
        _colors.sort([](const std::pair<float, Vector4>& a, const std::pair<float, Vector4>& b) { return a.first < b.first; });

    }
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    if (ImGui::BeginPopup("GradientEditor"))
    {
        ImGuiHelper::DrawTitleBar("GradientEditor");

        ImGuiHelper::DrawGradientEditor(_colors);

        ImGui::EndPopup();
    }


    ImGui::PopID();

}

// ===== タイムラインヘルパー関数 =====
namespace
{
using namespace ImGuiTool;
using namespace ImGuiTool::TimeLineConstants;

// スナップ適用＋ドラッグ状態リセット（重複排除）
void FinishKeyframeDrag(TimelineState& _state, SequenceEvent::KeyFrame& _keyFrame)
{
    if (_state.showSnapping)
    {
        int gridIndex = (int)round((double)_keyFrame.time / (double)_state.snappingInterval);
        _keyFrame.time = gridIndex * _state.snappingInterval;
    }
    _state.isDragging = false;
    _state.draggingKeyFrame = nullptr;
}

// コントロールパネル（再生/停止、時間表示、ズーム/スナップ、保存）
void DrawControlPanel(TimelineState& _state, AnimationSequence* _sequence, const ImVec2& _contentSize)
{
    ImVec2 controlPanelSize = ImVec2(_contentSize.x, kControlPanelHeight);
    ImGui::BeginChild("ControlPanel", controlPanelSize, true);

    float leftControlsWidth = 200.0f;
    float centerControlsWidth = 200.0f;

    // 再生/停止/巻き戻しボタン
    ImGui::BeginGroup();
    if (ImGui::Button("Play", ImVec2(45, 0)))
        _state.isPlaying = true;
    ImGui::SameLine();
    if (ImGui::Button("Stop", ImVec2(45, 0)))
        _state.isPlaying = false;
    ImGui::SameLine();
    if (ImGui::Button("Start", ImVec2(45, 0)))
    {
        _state.currentTime = 0.0f;
        _sequence->SetCurrentTime(_state.currentTime);
        _state.isPlaying = true;
    }
    ImGui::EndGroup();

    if (_state.isPlaying)
        _state.currentTime = _sequence->GetCurrent();

    // 現在時間
    ImGui::SameLine(leftControlsWidth);
    ImGui::BeginGroup();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Time:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::DragFloat("##CurrentTime", &_state.currentTime, 0.01f, 0.0f, _state.maxTime))
        _sequence->SetCurrentTime(_state.currentTime);
    ImGui::EndGroup();

    // ズームとスナップ
    ImGui::SameLine(leftControlsWidth + centerControlsWidth);
    ImGui::BeginGroup();
    if (ImGui::Button("-", ImVec2(20, 0)))
        _state.timeScale = std::max(20.0f, _state.timeScale - 20.0f);
    ImGui::SameLine(0, 2.0f);
    ImGui::Text("Zoom");
    ImGui::SameLine(0, 2.0f);
    if (ImGui::Button("+", ImVec2(20, 0)))
        _state.timeScale = std::min(500.0f, _state.timeScale + 20.0f);

    ImGui::SameLine(0, 20.0f);
    ImGui::Checkbox("Snap", &_state.showSnapping);
    ImGui::SameLine(0, 2.0f);
    ImGui::SetNextItemWidth(60.0f);
    ImGui::DragFloat("##SnapInterval", &_state.snappingInterval, 0.01f, 0.01f, 1.0f, "%.2f");
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(45, 0)))
        _sequence->Save();

    ImGui::EndChild();
}

// タイムスケールヘッダー（時間目盛り）
void DrawTimeScaleHeader(TimelineState& _state, ImDrawList* _drawList, const ImVec2& _timelineBase, const ImVec2& _contentSize)
{
    ImVec2 headerPos = ImVec2(_timelineBase.x + kTrackHeaderWidth, _timelineBase.y + kControlPanelHeight);
    ImVec2 headerSize = ImVec2(_contentSize.x - kTrackHeaderWidth - kValueDisplayWidth, kTimeScaleHeaderHeight);

    _drawList->AddRectFilled(headerPos, ImVec2(headerPos.x + headerSize.x, headerPos.y + headerSize.y), kTimelineBgColor);

    float pixelsPerSecond = _state.timeScale;
    float secondsInView = headerSize.x / pixelsPerSecond;
    _state.viewEnd = _state.viewStart + secondsInView;

    // 目盛り間隔を決定
    float majorTickInterval = 1.0f, minorTickInterval = 0.1f;
    if (secondsInView > 60.0f) { majorTickInterval = 10.0f; minorTickInterval = 1.0f; }
    else if (secondsInView > 20.0f) { majorTickInterval = 5.0f;  minorTickInterval = 1.0f; }
    else if (secondsInView > 10.0f) { majorTickInterval = 1.0f;  minorTickInterval = 0.5f; }

    // 小目盛り
    for (float t = floorf(_state.viewStart / minorTickInterval) * minorTickInterval; t <= _state.viewEnd; t += minorTickInterval)
    {
        float xPos = headerPos.x + (t - _state.viewStart) * pixelsPerSecond;
        if (xPos >= headerPos.x && xPos <= headerPos.x + headerSize.x)
        {
            bool isMajor = (fabs(roundf(t) - t) < 0.001f);
            float tickH = isMajor ? headerSize.y * 0.7f : headerSize.y * 0.4f;
            _drawList->AddLine(
                ImVec2(xPos, headerPos.y + headerSize.y - tickH),
                ImVec2(xPos, headerPos.y + headerSize.y),
                isMajor ? kTimelineGridMajorColor : kTimelineGridColor);
        }
    }

    // 大目盛り＋テキスト
    for (float t = floorf(_state.viewStart / majorTickInterval) * majorTickInterval; t <= _state.viewEnd; t += majorTickInterval)
    {
        float xPos = headerPos.x + (t - _state.viewStart) * pixelsPerSecond;
        if (xPos >= headerPos.x && xPos <= headerPos.x + headerSize.x)
        {
            _drawList->AddLine(ImVec2(xPos, headerPos.y), ImVec2(xPos, headerPos.y + headerSize.y), kTimelineGridMajorColor);

            char timeText[32];
            if (t < 60.0f)
                snprintf(timeText, sizeof(timeText), "%.1fs", t);
            else
                snprintf(timeText, sizeof(timeText), "%d:%05.2f", static_cast<int>(t) / 60, fmodf(t, 60.0f));

            _drawList->AddText(ImVec2(xPos + 3.0f, headerPos.y + 2.0f), IM_COL32(200, 200, 200, 255), timeText);
        }
    }
}

// グリッド＋現在時間マーカー＋最大再生時間マーカー
void DrawGridAndMarkers(const TimelineState& _state, AnimationSequence* _sequence, ImDrawList* _drawList,
                        const ImVec2& _tracksAreaPos, const ImVec2& _tracksAreaSize, float _tracksHeight)
{
    float pixelsPerSecond = _state.timeScale;

    // 垂直グリッドライン
    for (float t = floorf(_state.viewStart); t <= _state.viewEnd; t += 1.0f)
    {
        float xPos = _tracksAreaPos.x + kTrackHeaderWidth + (t - _state.viewStart) * pixelsPerSecond;
        if (xPos >= _tracksAreaPos.x + kTrackHeaderWidth && xPos <= _tracksAreaPos.x + _tracksAreaSize.x)
        {
            _drawList->AddLine(ImVec2(xPos, _tracksAreaPos.y), ImVec2(xPos, _tracksAreaPos.y + _tracksAreaSize.y), kTimelineGridMajorColor);
        }
    }

    // 現在時間の縦線＋逆三角マーカー
    float currentTimeX = _tracksAreaPos.x + kTrackHeaderWidth + (_state.currentTime - _state.viewStart) * pixelsPerSecond;
    if (currentTimeX >= _tracksAreaPos.x + kTrackHeaderWidth && currentTimeX <= _tracksAreaPos.x + _tracksAreaSize.x)
    {
        _drawList->AddLine(ImVec2(currentTimeX, _tracksAreaPos.y), ImVec2(currentTimeX, _tracksAreaPos.y + _tracksAreaSize.y), kCurrentTimeMarkerColor, 2.0f);

        ImVec2 tri[3] = {
            ImVec2(currentTimeX, _tracksAreaPos.y),
            ImVec2(currentTimeX - kCurrentTimeMarkerSize / 2, _tracksAreaPos.y - kCurrentTimeMarkerSize),
            ImVec2(currentTimeX + kCurrentTimeMarkerSize / 2, _tracksAreaPos.y - kCurrentTimeMarkerSize)
        };
        _drawList->AddConvexPolyFilled(tri, 3, kCurrentTimeMarkerColor);
    }

    // 最大再生時間マーカー（点線）
    float maxPlayTime = _sequence->GetMaxPlayTime();
    float maxPlayTimeX = _tracksAreaPos.x + kTrackHeaderWidth + (maxPlayTime - _state.viewStart) * pixelsPerSecond;
    if (maxPlayTimeX >= _tracksAreaPos.x + kTrackHeaderWidth && maxPlayTimeX <= _tracksAreaPos.x + _tracksAreaSize.x)
    {
        const int lineSegments = 20;
        const float dashLength = _tracksHeight / (lineSegments * 2);
        for (int i = 0; i < lineSegments; i++)
        {
            float startY = _tracksAreaPos.y + i * dashLength * 2;
            _drawList->AddLine(ImVec2(maxPlayTimeX, startY), ImVec2(maxPlayTimeX, startY + dashLength), IM_COL32(255, 100, 100, 200), 1.0f);
        }

        const char* label = "Max Time";
        ImVec2 labelSize = ImGui::CalcTextSize(label);
        _drawList->AddRectFilled(
            ImVec2(maxPlayTimeX - labelSize.x / 2 - 2, _tracksAreaPos.y - labelSize.y - 4),
            ImVec2(maxPlayTimeX + labelSize.x / 2 + 2, _tracksAreaPos.y),
            IM_COL32(80, 10, 10, 200));
        _drawList->AddText(
            ImVec2(maxPlayTimeX - labelSize.x / 2, _tracksAreaPos.y - labelSize.y - 2),
            IM_COL32(255, 200, 200, 255), label);
    }
}

// マウスがキーフレーム上にあるか判定
bool CheckMouseOverKeyframe(const std::list<SequenceEvent*>& _events, const TimelineState& _state,
                            const ImVec2& _tracksAreaPos, const ImVec2& _tracksAreaSize, const ImVec2& _mousePos)
{
    float pixelsPerSecond = _state.timeScale;
    float trackY = 0;
    for (auto event : _events)
    {
        for (auto& kf : event->GetKeyFrames())
        {
            float keyX = kTrackHeaderWidth + (kf.time - _state.viewStart) * pixelsPerSecond;
            if (keyX >= kTrackHeaderWidth && keyX <= _tracksAreaSize.x)
            {
                float dx = _mousePos.x - (_tracksAreaPos.x + keyX);
                float dy = _mousePos.y - (_tracksAreaPos.y + trackY + kTrackHeight / 2);
                float hitRadius = kTimeMarkerSize * 1.25f;
                if ((dx * dx + dy * dy) <= (hitRadius * hitRadius))
                    return true;
            }
        }
        trackY += kTrackHeight;
    }
    return false;
}

// タイムライン全体の入力処理（ドラッグスクロール/ズーム/中クリック）
void HandleTimelineInput(TimelineState& _state, AnimationSequence* _sequence,
                         const ImVec2& _tracksAreaPos, const ImVec2& _tracksAreaSize, float _mouseTime)
{
    float pixelsPerSecond = _state.timeScale;

    ImGui::SetCursorPos(ImVec2(kTrackHeaderWidth, 0));
    ImGui::InvisibleButton("TimelineDrag", ImVec2(_tracksAreaSize.x - kTrackHeaderWidth, _tracksAreaSize.y),
                           ImGuiButtonFlags_MouseButtonLeft);

    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        _state.viewStart -= ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
        _state.viewStart = std::max(0.0f, _state.viewStart);
    }

    if (ImGui::IsItemHovered())
    {
        float wheel = ImGui::GetIO().MouseWheel;
        if (wheel != 0)
        {
            float oldScale = _state.timeScale;
            _state.timeScale = (wheel > 0)
                ? std::min(500.0f, _state.timeScale * 1.2f)
                : std::max(20.0f, _state.timeScale / 1.2f);

            if (_state.timeScale != oldScale)
            {
                float mouseTimePos = _state.viewStart + (ImGui::GetMousePos().x - (_tracksAreaPos.x + kTrackHeaderWidth)) / oldScale;
                _state.viewStart = mouseTimePos - (ImGui::GetMousePos().x - (_tracksAreaPos.x + kTrackHeaderWidth)) / _state.timeScale;
                _state.viewStart = std::max(0.0f, _state.viewStart);
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && _mouseTime >= 0.0f)
        {
            _state.currentTime = _mouseTime;
            _sequence->SetCurrentTime(_state.currentTime);
        }
    }
}

// トラックヘッダー描画＋選択＋削除メニュー
void DrawTrackHeader(SequenceEvent* _event, const std::list<SequenceEvent*>& _allEvents,
                     ImDrawList* _drawList, const ImVec2& _tracksAreaPos, float _trackY)
{
    _drawList->AddRectFilled(
        ImVec2(_tracksAreaPos.x, _tracksAreaPos.y + _trackY),
        ImVec2(_tracksAreaPos.x + kTrackHeaderWidth, _tracksAreaPos.y + _trackY + kTrackHeight),
        _event->IsSelect() ? kTrackBgColorSelected : kTrackBgColor);

    _drawList->AddText(
        ImVec2(_tracksAreaPos.x + 5, _tracksAreaPos.y + _trackY + 2),
        IM_COL32(220, 220, 220, 255), _event->GetLabel().c_str());

    std::string btnId = "##Track_" + _event->GetLabel();
    ImGui::SetCursorPos(ImVec2(0, _trackY));
    ImGui::InvisibleButton(btnId.c_str(), ImVec2(kTrackHeaderWidth, kTrackHeight));

    if (ImGui::IsItemClicked())
    {
        for (auto other : _allEvents) { other->SetSelect(false); other->ClearSelectKeyFrames(); }
        _event->SetSelect(true);
    }

    std::string popupId = "TrackDeleteMenu" + _event->GetLabel();
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup(popupId.c_str());

    if (ImGui::BeginPopup(popupId.c_str()))
    {
        if (ImGui::Button("Delete Track"))
            _event->MarkForDelete();
        ImGui::EndPopup();
    }
}

// トラックの値表示
void DrawTrackValueDisplay(SequenceEvent* _event, AnimationSequence* _sequence, ImDrawList* _drawList,
                           const ImVec2& _valueDisplayPos, const ImVec2& _valueDisplaySize, float _trackY)
{
    ImU32 bgColor = _event->IsSelect() ? IM_COL32(70, 70, 90, 255) : IM_COL32(45, 45, 45, 255);
    _drawList->AddRectFilled(
        ImVec2(_valueDisplayPos.x, _valueDisplayPos.y + _trackY),
        ImVec2(_valueDisplayPos.x + _valueDisplaySize.x, _valueDisplayPos.y + _trackY + kTrackHeight),
        bgColor);

    _event->Update(_sequence->GetCurrent());

    char valueText[64] = "";
    std::visit([&](auto&& value)
               {
                   using T = std::decay_t<decltype(value)>;
                   if constexpr (std::is_same_v<T, int32_t>)
                       snprintf(valueText, sizeof(valueText), "%d", value);
                   else if constexpr (std::is_same_v<T, float>)
                       snprintf(valueText, sizeof(valueText), "%.2f", value);
                   else if constexpr (std::is_same_v<T, Vector2>)
                       snprintf(valueText, sizeof(valueText), "(%.1f, %.1f)", value.x, value.y);
                   else if constexpr (std::is_same_v<T, Vector3>)
                       snprintf(valueText, sizeof(valueText), "(%.1f, %.1f, %.1f)", value.x, value.y, value.z);
                   else if constexpr (std::is_same_v<T, Vector4> || std::is_same_v<T, Quaternion>)
                       snprintf(valueText, sizeof(valueText), "(%.1f, %.1f, %.1f, %.1f)", value.x, value.y, value.z, value.w);
               }, _event->GetValue());

    ImVec2 textSize = ImGui::CalcTextSize(valueText);
    _drawList->AddText(
        ImVec2(_valueDisplayPos.x + (_valueDisplaySize.x - textSize.x) / 2,
               _valueDisplayPos.y + _trackY + (kTrackHeight - textSize.y) / 2),
        IM_COL32(220, 220, 100, 255), valueText);
}

// キーフレーム描画＋ドラッグ＋コンテキストメニュー
void DrawKeyframes(TimelineState& _state, SequenceEvent* _event, const std::list<SequenceEvent*>& _allEvents,
                   ImDrawList* _drawList, const ImVec2& _tracksAreaPos, const ImVec2& _tracksAreaSize, float _trackY)
{
    float pixelsPerSecond = _state.timeScale;

    for (auto& keyFrame : _event->GetKeyFrames())
    {
        float keyX = kTrackHeaderWidth + (keyFrame.time - _state.viewStart) * pixelsPerSecond;
        float absKeyX = _tracksAreaPos.x + keyX;

        if (absKeyX >= _tracksAreaPos.x + kTrackHeaderWidth && absKeyX <= _tracksAreaPos.x + _tracksAreaSize.x)
        {
            float keyY = _trackY + kTrackHeight / 2;
            float absKeyY = _tracksAreaPos.y + keyY;

            ImGui::PushID((void*)&keyFrame);
            ImGui::SetCursorPos(ImVec2(keyX - kTimeMarkerSize, keyY - kTimeMarkerSize));

            std::string btnId = "##Key_" + _event->GetLabel() + "_" + std::to_string(keyFrame.time);
            ImGui::InvisibleButton(btnId.c_str(), ImVec2(kTimeMarkerSize * 2.5f, kTimeMarkerSize * 2.5f),
                                   ImGuiButtonFlags_MouseButtonLeft);

            // 選択処理
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                for (auto& other : _allEvents) other->ClearSelectKeyFrames();
                keyFrame.isSelect = true;
            }

            // 色決定
            ImU32 keyColor = kKeyframeColor;
            if (ImGui::IsItemHovered() && !_state.isDragging)
                keyColor = kKeyframeColorHover;
            else if (keyFrame.isSelect)
                keyColor = kKeyframeColorSelected;

            _drawList->AddCircleFilled(ImVec2(absKeyX, absKeyY), kTimeMarkerSize, keyColor);

            // ドラッグ開始
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                keyFrame.isSelect = true;
                _state.isDragging = true;
                _state.draggingKeyFrame = &keyFrame;
            }

            // ドラッグ中の処理
            if (_state.isDragging && _state.draggingKeyFrame == &keyFrame)
            {
                float newTime = std::max(0.0f, keyFrame.time + ImGui::GetIO().MouseDelta.x / pixelsPerSecond);
                keyFrame.time = newTime;

                if (_state.showSnapping)
                {
                    float snappedTime = roundf(newTime / _state.snappingInterval) * _state.snappingInterval;
                    float snapX = _tracksAreaPos.x + kTrackHeaderWidth + (snappedTime - _state.viewStart) * pixelsPerSecond;
                    _drawList->AddLine(
                        ImVec2(snapX, _tracksAreaPos.y), ImVec2(snapX, _tracksAreaPos.y + _tracksAreaSize.y),
                        IM_COL32(255, 255, 0, 100), 1.0f);
                }
                _event->SortKeyFrames();
            }

            // ドラッグ終了
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && _state.isDragging && _state.draggingKeyFrame == &keyFrame)
                FinishKeyframeDrag(_state, keyFrame);

            // 右クリックメニュー
            ImGui::SetCursorPos(ImVec2(keyX - kTimeMarkerSize, keyY - kTimeMarkerSize));
            ImGui::InvisibleButton("keyframe_rmb", ImVec2(kTimeMarkerSize * 2.5f, kTimeMarkerSize * 2.5f),
                                   ImGuiButtonFlags_MouseButtonRight);

            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                ImGui::OpenPopup("KeyContextMenu");
                keyFrame.isSelect = true;
            }

            if (ImGui::BeginPopup("KeyContextMenu"))
            {
                if (ImGui::MenuItem("Delete"))
                    keyFrame.isDelete = true;
                if (ImGui::BeginMenu("Edit Value"))
                {
                    SequenceEvent::EditKeyFrameValue(keyFrame);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Easing"))
                {
                    keyFrame.easingType = Easing::SelectEasingFunc(keyFrame.easingType);
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
        // 範囲外のドラッグ中キーフレーム → 解除
        else if (_state.isDragging && _state.draggingKeyFrame == &keyFrame)
        {
            FinishKeyframeDrag(_state, keyFrame);
        }
    }
}

// メインのトラック領域全体
void DrawTracksArea(TimelineState& _state, AnimationSequence* _sequence, ImDrawList* _drawList,
                    const ImVec2& _windowPos, const ImVec2& _contentSize)
{
    ImVec2 timelineBase = ImVec2(_windowPos.x, _windowPos.y + kTimelineHeightPadding);
    float tracksStartY = timelineBase.y + kControlPanelHeight + kTimeScaleHeaderHeight;
    float tracksHeight = _contentSize.y - kControlPanelHeight - kTimeScaleHeaderHeight - kStatusBarHeight - kTimelineHeightPadding;
    if (tracksHeight <= 0.0f) tracksHeight = 1.0f;

    ImVec2 tracksAreaPos = ImVec2(_windowPos.x, tracksStartY);
    ImVec2 tracksAreaSize = ImVec2(_contentSize.x - kValueDisplayWidth, tracksHeight);
    ImVec2 valueDisplayPos = ImVec2(tracksAreaPos.x + tracksAreaSize.x, tracksAreaPos.y);
    ImVec2 valueDisplaySize = ImVec2(kValueDisplayWidth, tracksHeight);

    // 背景描画
    _drawList->AddRectFilled(tracksAreaPos, ImVec2(tracksAreaPos.x + tracksAreaSize.x, tracksAreaPos.y + tracksAreaSize.y), kTimelineBgColor);
    _drawList->AddRectFilled(valueDisplayPos, ImVec2(valueDisplayPos.x + valueDisplaySize.x, valueDisplayPos.y + valueDisplaySize.y), IM_COL32(45, 45, 45, 255));
    _drawList->AddRectFilled(tracksAreaPos, ImVec2(tracksAreaPos.x + kTrackHeaderWidth, tracksAreaPos.y + tracksAreaSize.y), IM_COL32(55, 55, 55, 255));

    // 値表示ヘッダー
    const char* valueHeader = "Current Value";
    ImVec2 headerTextSize = ImGui::CalcTextSize(valueHeader);
    _drawList->AddText(
        ImVec2(valueDisplayPos.x + (valueDisplaySize.x - headerTextSize.x) / 2,
               valueDisplayPos.y - kTimeScaleHeaderHeight + (kTimeScaleHeaderHeight - headerTextSize.y) / 2),
        IM_COL32(200, 200, 200, 255), valueHeader);

    DrawGridAndMarkers(_state, _sequence, _drawList, tracksAreaPos, tracksAreaSize, tracksHeight);

    auto sequenceEvents = _sequence->GetSequenceEvents();
    float pixelsPerSecond = _state.timeScale;

    // Child開始
    float childPosY = tracksStartY - _windowPos.y;
    ImGui::SetCursorPos(ImVec2(0, childPosY));
    ImGui::BeginChild("TracksArea", tracksAreaSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    // マウス時間の計算
    ImVec2 mousePos = ImGui::GetMousePos();
    float mouseTime = -1.0f;
    if (mousePos.x >= tracksAreaPos.x + kTrackHeaderWidth && mousePos.x <= tracksAreaPos.x + tracksAreaSize.x &&
        mousePos.y >= tracksAreaPos.y && mousePos.y <= tracksAreaPos.y + tracksAreaSize.y)
    {
        mouseTime = _state.viewStart + (mousePos.x - (tracksAreaPos.x + kTrackHeaderWidth)) / pixelsPerSecond;
        if (_state.showSnapping)
        {
            float snapped = roundf(mouseTime / _state.snappingInterval) * _state.snappingInterval;
            if (fabsf(mouseTime - snapped) < kSnapThreshold)
                mouseTime = snapped;
        }
    }

    bool isMouseOverKF = CheckMouseOverKeyframe(sequenceEvents, _state, tracksAreaPos, tracksAreaSize, mousePos);

    // タイムライン入力（キーフレーム上でない場合のみ）
    if (!isMouseOverKF)
        HandleTimelineInput(_state, _sequence, tracksAreaPos, tracksAreaSize, mouseTime);

    // 右クリックでキーフレーム追加
    if (!isMouseOverKF && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && mouseTime >= 0.0f)
    {
        _state.trackIndex = floorf((mousePos.y - tracksAreaPos.y) / kTrackHeight);
        if (_state.trackIndex >= 0 && _state.trackIndex < static_cast<float>(sequenceEvents.size()))
        {
            auto it = sequenceEvents.begin();
            std::advance(it, static_cast<int>(_state.trackIndex));
            if (*it) ImGui::OpenPopup("TrackContextMenu");
        }
    }

    if (ImGui::BeginPopup("TrackContextMenu"))
    {
        if (ImGui::MenuItem("Add Keyframe"))
        {
            if (_state.trackIndex >= 0 && _state.trackIndex < static_cast<float>(sequenceEvents.size()))
            {
                auto it = sequenceEvents.begin();
                std::advance(it, static_cast<int>(_state.trackIndex));
                if (*it) { (*it)->AddKeyFrame(mouseTime); _state.trackIndex = 0.0f; }
            }
        }
        ImGui::EndPopup();
    }

    // 各トラック描画
    float trackY = 0;
    for (auto sequenceEvent : sequenceEvents)
    {
        sequenceEvent->DeleteMarkedKeyFrame();

        // トラック本体部分の背景
        _drawList->AddRectFilled(
            ImVec2(tracksAreaPos.x + kTrackHeaderWidth, tracksAreaPos.y + trackY),
            ImVec2(tracksAreaPos.x + tracksAreaSize.x, tracksAreaPos.y + trackY + kTrackHeight),
            sequenceEvent->IsSelect() ? IM_COL32(60, 60, 70, 100) : IM_COL32(50, 50, 50, 80));

        DrawTrackHeader(sequenceEvent, sequenceEvents, _drawList, tracksAreaPos, trackY);
        DrawTrackValueDisplay(sequenceEvent, _sequence, _drawList, valueDisplayPos, valueDisplaySize, trackY);
        DrawKeyframes(_state, sequenceEvent, sequenceEvents, _drawList, tracksAreaPos, tracksAreaSize, trackY);

        trackY += kTrackHeight;
    }

    ImGui::EndChild();
}

// ステータスバー
void DrawStatusBar(const TimelineState& _state, AnimationSequence* _sequence, ImDrawList* _drawList,
                   const ImVec2& _windowPos, const ImVec2& _windowSize)
{
    float statusBarY = _windowPos.y + _windowSize.y - kStatusBarHeight;
    ImVec2 pos = ImVec2(_windowPos.x, statusBarY);

    _drawList->AddRectFilled(pos, ImVec2(pos.x + _windowSize.x, pos.y + kStatusBarHeight), IM_COL32(40, 40, 40, 255));

    char text[128];
    snprintf(text, sizeof(text), "Time: %.2fs | Scale: %.0f | Tracks: %zu",
             _state.currentTime, _state.timeScale, _sequence->GetSequenceEvents().size());
    _drawList->AddText(ImVec2(pos.x + 5, pos.y + 2), IM_COL32(200, 200, 200, 255), text);
}

// 再生ロジック
void UpdatePlayback(TimelineState& _state, AnimationSequence* _sequence)
{
    if (!_state.isPlaying) return;

    const float deltaTime = 1.0f / 60.0f;
    _state.currentTime += deltaTime;

    if (_sequence->IsLooping())
    {
        if (_state.currentTime >= _sequence->GetMaxPlayTime())
            _state.currentTime = 0.0f;
    }
    else
    {
        if (_state.currentTime >= _sequence->GetMaxPlayTime())
        {
            _state.currentTime = _sequence->GetMaxPlayTime();
            _state.isPlaying = false;
        }
    }

    _sequence->SetCurrentTime(_state.currentTime);
    _sequence->Update(deltaTime);
}

// 下部コントロール（New Event, Loop, Max Time）
void DrawBottomControls(AnimationSequence* _sequence, const ImVec2& _windowSize)
{
    ImGui::SetCursorPos(ImVec2(5, _windowSize.y - kStatusBarHeight - 30));

    if (ImGui::Button("New Event"))
        ImGui::OpenPopup("NewEventPopup");

    float rightStart = 150.0f;
    ImGui::SameLine(rightStart);

    bool isLooping = _sequence->IsLooping();
    if (ImGui::Checkbox("Loop", &isLooping))
        _sequence->SetLooping(isLooping);

    ImGui::SameLine(rightStart + 100.0f);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Max Time:");
    ImGui::SameLine();

    float maxPlayTime = _sequence->GetMaxPlayTime();
    ImGui::SetNextItemWidth(120.0f);
    if (ImGui::DragFloat("##MaxPlayTime", &maxPlayTime, 0.01f, 0.1f, 1000.0f, "%.1f s"))
        _sequence->SetMaxPlayTime(maxPlayTime);

    if (ImGui::BeginPopup("NewEventPopup"))
    {
        static char newEventName[64] = "";
        static int selectedType = 0;
        static const char* eventTypes[] = { "int32_t","float", "Vector2", "Vector3", "Vector4","Quaternion" };

        ImGui::InputText("Event Name", newEventName, sizeof(newEventName));
        ImGui::Combo("Type", &selectedType, eventTypes, IM_ARRAYSIZE(eventTypes));

        if (ImGui::Button("Create") && strlen(newEventName) > 0)
        {
            switch (selectedType)
            {
                case 0: _sequence->CreateSequenceEvent<int32_t>(newEventName, int32_t(0)); break;
                case 1: _sequence->CreateSequenceEvent<float>(newEventName, 0.0f); break;
                case 2: _sequence->CreateSequenceEvent<Vector2>(newEventName, Vector2(0, 0)); break;
                case 3: _sequence->CreateSequenceEvent<Vector3>(newEventName, Vector3(0, 0, 0)); break;
                case 4: _sequence->CreateSequenceEvent<Vector4>(newEventName, Vector4(0, 0, 0, 0)); break;
                case 5: _sequence->CreateSequenceEvent<Quaternion>(newEventName, Quaternion(0, 0, 0, 1).Normalize()); break;
            }
            newEventName[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

} // namespace

void ImGuiTool::TimeLine(const char* _label, AnimationSequence* _sequence)
{
    if (!_sequence) return;
    _sequence->DeleteMarkedSequenceEvent();

    static TimelineState state;

    ImGui::PushID(_label);
    ImGui::Begin(_label, nullptr, ImGuiWindowFlags_NoScrollbar);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImVec2 timelineBase = ImVec2(windowPos.x, windowPos.y + TimeLineConstants::kTimelineHeightPadding);

    DrawControlPanel(state, _sequence, contentSize);
    DrawTimeScaleHeader(state, drawList, timelineBase, contentSize);
    DrawTracksArea(state, _sequence, drawList, windowPos, contentSize);
    DrawStatusBar(state, _sequence, drawList, windowPos, windowSize);
    UpdatePlayback(state, _sequence);
    DrawBottomControls(_sequence, windowSize);

    ImGui::End();
    ImGui::PopID();
}


#endif // _DEBUG
