#define NOMINMAX
#include "ImguITools.h"

#include <Debug/ImGuiHelper.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <Features/Animation/Sequence/AnimationSequence.h>

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
    using namespace TimeLineConstants;

    // Nullチェック
    if (!_sequence)
    {
        return;
    }

    ImGui::PushID(_label);

    // タイムラインウィンドウの設定
    ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoScrollbar);

    // タイムラインウィンドウの情報
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    ImVec2 contentSize = ImGui::GetContentRegionAvail();

    // タイムラインのベース位置
    ImVec2 timelineBase = windowPos;
    timelineBase.y += kTimelineHeightPadding;

    // 現在の時間と最大時間
    static float currentTime = _sequence->GetCurrent();
    static float maxTime = 10.0f; // デフォルトの最大時間
    static float viewStart = 0.0f; // 表示開始時間
    static float viewEnd = maxTime; // 表示終了時間
    static float timeScale = kTimeScale; // 時間スケール（ズーム）
    static bool showSnapping = true; // スナッピング機能の有効/無効
    static float snappingInterval = 0.1f; // スナップ間隔（秒）

    // ===== コントロールパネル部分 =====
    {
        // コントロールパネル全体のサイズを固定
        ImVec2 controlPanelSize = ImVec2(contentSize.x, kControlPanelHeight);
        ImGui::BeginChild("ControlPanel", controlPanelSize, true);

        // 固定幅レイアウトを使用
        float panelWidth = ImGui::GetContentRegionAvail().x;
        float leftControlsWidth = 200.0f;  // 左側のコントロール幅
        float centerControlsWidth = 200.0f; // 中央のコントロール幅
        float rightControlsWidth = 200.0f;  // 右側のコントロール幅

        // 左側のコントロール：再生/停止/巻き戻しボタン
        ImGui::BeginGroup();
        if (ImGui::Button("Play", ImVec2(45, 0))) {
            // 再生ロジック
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop", ImVec2(45, 0))) {
            // 停止ロジック
        }
        ImGui::SameLine();
        if (ImGui::Button("Start", ImVec2(45, 0))) {
            currentTime = 0.0f;
            _sequence->SetCurrentTime(currentTime);
        }
        ImGui::EndGroup();

        // 中央のコントロール：現在時間
        ImGui::SameLine(leftControlsWidth);
        ImGui::BeginGroup();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Time:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat("##CurrentTime", &currentTime, 0.01f, 0.0f, maxTime)) {
            _sequence->SetCurrentTime(currentTime);
        }
        ImGui::EndGroup();

        // 右側のコントロール：ズームとスナップ
        ImGui::SameLine(leftControlsWidth + centerControlsWidth);
        ImGui::BeginGroup();

        // ズーム
        if (ImGui::Button("-", ImVec2(20, 0))) {
            timeScale = std::max(20.0f, timeScale - 20.0f);
        }
        ImGui::SameLine(0, 2.0f);
        ImGui::Text("Zoom");
        ImGui::SameLine(0, 2.0f);
        if (ImGui::Button("+", ImVec2(20, 0))) {
            timeScale = std::min(500.0f, timeScale + 20.0f);
        }


        // スナップ
        ImGui::SameLine(0, 20.0f); // ズームコントロールとの間隔
        ImGui::Checkbox("Snap", &showSnapping);
        ImGui::SameLine(0, 2.0f);
        ImGui::SetNextItemWidth(60.0f);
        ImGui::DragFloat("##SnapInterval", &snappingInterval, 0.01f, 0.01f, 1.0f, "%.2f");
        ImGui::EndGroup();

        ImGui::EndChild();
    }

    // ===== タイムスケールヘッダー部分 =====
    {
        ImVec2 timeScaleHeaderPos = ImVec2(windowPos.x + kTrackHeaderWidth, timelineBase.y + kControlPanelHeight);
        ImVec2 timeScaleHeaderSize = ImVec2(contentSize.x - kTrackHeaderWidth, kTimeScaleHeaderHeight);

        // 時間軸の背景を描画
        drawList->AddRectFilled(
            timeScaleHeaderPos,
            ImVec2(timeScaleHeaderPos.x + timeScaleHeaderSize.x, timeScaleHeaderPos.y + timeScaleHeaderSize.y),
            kTimelineBgColor
        );

        // 時間目盛りを描画
        float pixelsPerSecond = timeScale;
        float secondsInView = timeScaleHeaderSize.x / pixelsPerSecond;
        viewEnd = viewStart + secondsInView;

        // 秒数に応じて目盛りの間隔を調整
        float majorTickInterval = 1.0f; // 秒単位の大目盛り
        float minorTickInterval = 0.1f; // 秒単位の小目盛り

        if (secondsInView > 60.0f) {
            majorTickInterval = 10.0f;
            minorTickInterval = 1.0f;
        }
        else if (secondsInView > 20.0f) {
            majorTickInterval = 5.0f;
            minorTickInterval = 1.0f;
        }
        else if (secondsInView > 10.0f) {
            majorTickInterval = 1.0f;
            minorTickInterval = 0.5f;
        }

        // 小目盛りを描画
        for (float t = floorf(viewStart / minorTickInterval) * minorTickInterval; t <= viewEnd; t += minorTickInterval) {
            float xPos = timeScaleHeaderPos.x + (t - viewStart) * pixelsPerSecond;

            if (xPos >= timeScaleHeaderPos.x && xPos <= timeScaleHeaderPos.x + timeScaleHeaderSize.x) {
                // 整数秒の場合は少し長い目盛りに
                bool isMajorTick = (fabs(roundf(t) - t) < 0.001f);
                float tickHeight = isMajorTick ? timeScaleHeaderSize.y * 0.7f : timeScaleHeaderSize.y * 0.4f;

                drawList->AddLine(
                    ImVec2(xPos, timeScaleHeaderPos.y + timeScaleHeaderSize.y - tickHeight),
                    ImVec2(xPos, timeScaleHeaderPos.y + timeScaleHeaderSize.y),
                    isMajorTick ? kTimelineGridMajorColor : kTimelineGridColor
                );
            }
        }

        // 大目盛りと秒数テキストを描画
        for (float t = floorf(viewStart / majorTickInterval) * majorTickInterval; t <= viewEnd; t += majorTickInterval) {
            float xPos = timeScaleHeaderPos.x + (t - viewStart) * pixelsPerSecond;

            if (xPos >= timeScaleHeaderPos.x && xPos <= timeScaleHeaderPos.x + timeScaleHeaderSize.x) {
                // 大目盛り線を描画
                drawList->AddLine(
                    ImVec2(xPos, timeScaleHeaderPos.y),
                    ImVec2(xPos, timeScaleHeaderPos.y + timeScaleHeaderSize.y),
                    kTimelineGridMajorColor
                );

                // 秒数テキストを描画
                char timeText[32];
                if (t < 60.0f) {
                    snprintf(timeText, sizeof(timeText), "%.1fs", t);
                }
                else {
                    int minutes = static_cast<int>(t) / 60;
                    float seconds = fmodf(t, 60.0f);
                    snprintf(timeText, sizeof(timeText), "%d:%05.2f", minutes, seconds);
                }

                drawList->AddText(
                    ImVec2(xPos + 3.0f, timeScaleHeaderPos.y + 2.0f),
                    IM_COL32(200, 200, 200, 255),
                    timeText
                );
            }
        }
    }

    // ===== メインのタイムライントラック領域 =====
    {
        float tracksStartY = timelineBase.y + kControlPanelHeight + kTimeScaleHeaderHeight;
        float tracksHeight = contentSize.y - kControlPanelHeight - kTimeScaleHeaderHeight - kStatusBarHeight - kTimelineHeightPadding;

        ImVec2 tracksAreaPos = ImVec2(windowPos.x, tracksStartY);
        ImVec2 tracksAreaSize = ImVec2(contentSize.x, tracksHeight);

        // トラックエリアの背景
        drawList->AddRectFilled(
            tracksAreaPos,
            ImVec2(tracksAreaPos.x + tracksAreaSize.x, tracksAreaPos.y + tracksAreaSize.y),
            kTimelineBgColor
        );

        // ヘッダー部分の背景
        drawList->AddRectFilled(
            tracksAreaPos,
            ImVec2(tracksAreaPos.x + kTrackHeaderWidth, tracksAreaPos.y + tracksAreaSize.y),
            IM_COL32(55, 55, 55, 255)
        );

        // 垂直グリッドラインを描画
        float pixelsPerSecond = timeScale;
        for (float t = floorf(viewStart); t <= viewEnd; t += 1.0f) {
            float xPos = tracksAreaPos.x + kTrackHeaderWidth + (t - viewStart) * pixelsPerSecond;

            if (xPos >= tracksAreaPos.x + kTrackHeaderWidth && xPos <= tracksAreaPos.x + tracksAreaSize.x) {
                drawList->AddLine(
                    ImVec2(xPos, tracksAreaPos.y),
                    ImVec2(xPos, tracksAreaPos.y + tracksAreaSize.y),
                    kTimelineGridMajorColor
                );
            }
        }

        // 現在時間の縦線を描画
        float currentTimeX = tracksAreaPos.x + kTrackHeaderWidth + (currentTime - viewStart) * pixelsPerSecond;
        if (currentTimeX >= tracksAreaPos.x + kTrackHeaderWidth && currentTimeX <= tracksAreaPos.x + tracksAreaSize.x) {
            drawList->AddLine(
                ImVec2(currentTimeX, tracksAreaPos.y),
                ImVec2(currentTimeX, tracksAreaPos.y + tracksAreaSize.y),
                kCurrentTimeMarkerColor,
                2.0f
            );

            // トップマーカー（逆三角形）の描画
            ImVec2 trianglePoints[3] = {
                ImVec2(currentTimeX, tracksAreaPos.y),
                ImVec2(currentTimeX - kCurrentTimeMarkerSize / 2, tracksAreaPos.y - kCurrentTimeMarkerSize),
                ImVec2(currentTimeX + kCurrentTimeMarkerSize / 2, tracksAreaPos.y - kCurrentTimeMarkerSize)
            };
            drawList->AddConvexPolyFilled(trianglePoints, 3, kCurrentTimeMarkerColor);
        }

        // シーケンスイベントの表示
        auto sequenceEvents = _sequence->GetSequenceEvents();

        // トラック領域のスクロール用Child作成
        // Child領域の位置をトラックエリアの位置に正確に合わせる
        float childPosY = tracksStartY - windowPos.y;
        ImGui::SetCursorPos(ImVec2(0, childPosY));
        ImGui::BeginChild("TracksArea", tracksAreaSize, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

        // マウス位置での時間を取得（トラックエリア内のみ）
        ImVec2 mousePos = ImGui::GetMousePos();
        float mouseTime = -1.0f;
        if (mousePos.x >= tracksAreaPos.x + kTrackHeaderWidth && mousePos.x <= tracksAreaPos.x + tracksAreaSize.x &&
            mousePos.y >= tracksAreaPos.y && mousePos.y <= tracksAreaPos.y + tracksAreaSize.y) {
            mouseTime = viewStart + (mousePos.x - (tracksAreaPos.x + kTrackHeaderWidth)) / pixelsPerSecond;

            // スナッピング処理
            if (showSnapping) {
                float snappedTime = roundf(mouseTime / snappingInterval) * snappingInterval;
                if (fabsf(mouseTime - snappedTime) < kSnapThreshold) {
                    mouseTime = snappedTime;
                }
            }
        }

        // ===== キーフレームウィジェットが存在するかどうかを判断するフラグ =====
        bool isMouseOverKeyframe = false;

        // 前処理: マウスが現在キーフレーム上にあるか確認
        float trackY = 0;
        for (auto sequenceEvent : sequenceEvents) {
            for (auto& keyFrame : sequenceEvent->GetKeyFrames()) {
                float keyTime = keyFrame.time;
                float keyX = kTrackHeaderWidth + (keyTime - viewStart) * pixelsPerSecond;

                if (keyX >= kTrackHeaderWidth && keyX <= tracksAreaSize.x) {
                    float keyY = trackY + kTrackHeight / 2;

                    // マウスがキーフレーム上にあるかチェック
                    float dx = mousePos.x - (tracksAreaPos.x + keyX);
                    float dy = mousePos.y - (tracksAreaPos.y + keyY);
                    float hitRadius = kTimeMarkerSize * 1.25f;

                    if ((dx * dx + dy * dy) <= (hitRadius * hitRadius)) {
                        isMouseOverKeyframe = true;
                        break;
                    }
                }
            }
            if (isMouseOverKeyframe) break;
            trackY += kTrackHeight;
        }

        // タイムライン全体のドラッグ操作（キーフレーム上にない場合のみ左ボタンドラッグを許可）
        if (!isMouseOverKeyframe) {
            ImGui::SetCursorPos(ImVec2(kTrackHeaderWidth, 0));
            ImGui::InvisibleButton("TimelineDrag", ImVec2(tracksAreaSize.x - kTrackHeaderWidth, tracksAreaSize.y),
                ImGuiButtonFlags_MouseButtonLeft);

            // 左ボタンドラッグでタイムラインをスクロール
            if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                viewStart -= ImGui::GetIO().MouseDelta.x / pixelsPerSecond;
                viewStart = std::max(0.0f, viewStart);
            }

            // タイムラインのズーム（マウスホイール回転）
            if (ImGui::IsItemHovered()) {
                if (ImGui::GetIO().MouseWheel != 0) {
                    float oldTimeScale = timeScale;
                    if (ImGui::GetIO().MouseWheel > 0) {
                        timeScale = std::min(500.0f, timeScale * 1.2f);
                    }
                    else {
                        timeScale = std::max(20.0f, timeScale / 1.2f);
                    }

                    // ズーム中心を保持するための調整
                    if (timeScale != oldTimeScale) {
                        float mouseTimePos = viewStart + (ImGui::GetMousePos().x - (tracksAreaPos.x + kTrackHeaderWidth)) / oldTimeScale;
                        viewStart = mouseTimePos - (ImGui::GetMousePos().x - (tracksAreaPos.x + kTrackHeaderWidth)) / timeScale;
                        viewStart = std::max(0.0f, viewStart);
                    }
                }
            }

            // 中クリックで現在時間を変更
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle) && mouseTime >= 0.0f) {
                currentTime = mouseTime;
                _sequence->SetCurrentTime(currentTime);
            }
        }

        static float trackIndex = 0.0f;
        // マウスの右クリックで新しいキーフレームを追加
        if (!isMouseOverKeyframe && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && mouseTime >= 0.0f) {
            trackIndex = floorf((mousePos.y - tracksAreaPos.y) / kTrackHeight);
            if (trackIndex >= 0 && trackIndex < static_cast<float>(sequenceEvents.size())) {
                auto it = sequenceEvents.begin();
                std::advance(it, static_cast<int>(trackIndex));
                if (*it) {
                    // 右クリックメニューを開く
                    ImGui::OpenPopup("TrackContextMenu");
                }
            }
        }

        // 右クリックメニュー
        if (ImGui::BeginPopup("TrackContextMenu")) {
            if (ImGui::MenuItem("Add Keyframe")) {
                if (trackIndex >= 0 && trackIndex < static_cast<float>(sequenceEvents.size())) {
                    auto it = sequenceEvents.begin();
                    std::advance(it, static_cast<int>(trackIndex));
                    if (*it) {
                        SequenceEvent* event = *it;
                        // 該当のトラックにキーフレームを追加
                        event->AddKeyFrame(mouseTime, 0.0f, 0); // 仮の値
                        trackIndex = 0.0f;
                    }
                }
            }
            ImGui::EndPopup();
        }

        // 各トラックを描画
        trackY = 0; // Child内の相対位置
        for (auto sequenceEvent : sequenceEvents) {
            // トラックの背景を描画（Childウィンドウ内の相対位置で計算）
            ImVec2 trackStartPos = ImVec2(0, trackY);
            ImVec2 trackEndPos = ImVec2(tracksAreaSize.x, trackY + kTrackHeight);

            // トラックヘッダー部分
            drawList->AddRectFilled(
                ImVec2(tracksAreaPos.x, tracksAreaPos.y + trackY),
                ImVec2(tracksAreaPos.x + kTrackHeaderWidth, tracksAreaPos.y + trackY + kTrackHeight),
                sequenceEvent->IsSelect() ? kTrackBgColorSelected : kTrackBgColor
            );

            // トラック名を描画
            drawList->AddText(
                ImVec2(tracksAreaPos.x + 5, tracksAreaPos.y + trackY + 2),
                IM_COL32(220, 220, 220, 255),
                sequenceEvent->GetLabel().c_str()
            );

            // トラック本体部分
            drawList->AddRectFilled(
                ImVec2(tracksAreaPos.x + kTrackHeaderWidth, tracksAreaPos.y + trackY),
                ImVec2(tracksAreaPos.x + tracksAreaSize.x, tracksAreaPos.y + trackY + kTrackHeight),
                sequenceEvent->IsSelect() ? IM_COL32(60, 60, 70, 100) : IM_COL32(50, 50, 50, 80)
            );

            // トラックの選択用インビジブルボタン（Childウィンドウ内の相対位置）
            std::string trackButtonId = "##Track_" + sequenceEvent->GetLabel();
            ImGui::SetCursorPos(ImVec2(0, trackY));
            ImGui::InvisibleButton(trackButtonId.c_str(), ImVec2(kTrackHeaderWidth, kTrackHeight));

            if (ImGui::IsItemClicked()) {
                // 他のトラックの選択を解除
                for (auto otherEvent : sequenceEvents) {
                    otherEvent->SetSelect(false);
                    otherEvent->ClearSelectKeyFrames();
                }
                // このトラックを選択
                sequenceEvent->SetSelect(true);
            }

            static bool isDragging = false;
            static SequenceEvent::KeyFrame* draggingKeyFrame = nullptr;

            // キーフレームを描画
            for (auto& keyFrame : sequenceEvent->GetKeyFrames()) {
                float keyTime = keyFrame.time;
                float keyX = kTrackHeaderWidth + (keyTime - viewStart) * pixelsPerSecond; // Child内の相対X座標
                float absKeyX = tracksAreaPos.x + keyX; // 画面上の絶対X座標

                // 表示範囲内のキーフレームのみ描画
                if (absKeyX >= tracksAreaPos.x + kTrackHeaderWidth && absKeyX <= tracksAreaPos.x + tracksAreaSize.x) {
                    float keyY = trackY + kTrackHeight / 2; // Child内の相対Y座標
                    float absKeyY = tracksAreaPos.y + keyY; // 画面上の絶対Y座標

                    // キーフレーム用のID範囲を設定
                    ImGui::PushID((void*)&keyFrame);

                    // カーソル位置を設定
                    ImGui::SetCursorPos(ImVec2(keyX - kTimeMarkerSize, keyY - kTimeMarkerSize));

                    std::string keyButtonId = "##Key_" + sequenceEvent->GetLabel() + "_" + std::to_string(keyFrame.time);

                    // キーフレーム選択とドラッグは左クリックのみで処理
                    ImGui::InvisibleButton(keyButtonId.c_str(), ImVec2(kTimeMarkerSize * 2.5f, kTimeMarkerSize * 2.5f),
                        ImGuiButtonFlags_MouseButtonLeft);

                    // 左クリックでの選択処理
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        // 他のキーフレームの選択をクリア
                        for (auto& otherEvent : sequenceEvents) {
                            otherEvent->ClearSelectKeyFrames();
                            }
                        keyFrame.isSelect = true;
                    }

                    // キーフレームの色を決定
                    ImU32 keyColor = kKeyframeColor; // デフォルト色


                    if (ImGui::IsItemHovered() && !isDragging) {
                        keyColor = kKeyframeColorHover; // ホバー状態
                    }
                    else if (keyFrame.isSelect) {
                        keyColor = kKeyframeColorSelected; // 選択状態
                    }

                    // キーフレームマーカーを描画
                    drawList->AddCircleFilled(
                        ImVec2(absKeyX, absKeyY),
                        kTimeMarkerSize,
                        keyColor
                    );


                    // TODO
                    bool isActive = ImGui::IsItemActive();
                   /* ImGui::Begin("test");

                    ImGui::Text("Hover :%s", ImGui::IsItemHovered() ? "true" : "false");
                    ImGui::Text("Active :%s", isActive ? "true" : "false");
                    ImGui::Text("Clicked :%s", ImGui::IsItemClicked(ImGuiMouseButton_Left) ? "true" : "false");
                    ImGui::Text("Select :%s", keyFrame.isSelect ? "true" : "false");
                    ImGui::Text("isDrag :%s", ImGui::IsMouseDragging(ImGuiMouseButton_Left) ? "true" : "false");

                    ImGui::End();*/



                    // ドラッグ処理
                    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                        keyFrame.isSelect = true;
                        isDragging = true;
                        draggingKeyFrame = &keyFrame;
                    }

                    if (isDragging && draggingKeyFrame == &keyFrame) {
                        float newTime = keyTime + ImGui::GetIO().MouseDelta.x / pixelsPerSecond;

                        // 範囲制限
                        newTime = std::max(0.0f, newTime);
                        keyFrame.time = newTime;

                        // ドラッグ中のスナップガイド表示
                        if (showSnapping) {
                            // 最も近いスナップ位置を計算
                            float snappedTime = roundf(newTime / snappingInterval) * snappingInterval;
                            //if (fabsf(newTime - snappedTime) < kSnapThreshold) {
                                // スナップガイドラインを描画
                                float snapX = tracksAreaPos.x + kTrackHeaderWidth + (snappedTime - viewStart) * pixelsPerSecond;
                                drawList->AddLine(
                                    ImVec2(snapX, tracksAreaPos.y),
                                    ImVec2(snapX, tracksAreaPos.y + tracksAreaSize.y),
                                    IM_COL32(255, 255, 0, 100), // 黄色の半透明ライン
                                    1.0f
                                );
                            //}
                        }
                    }

                    // マウスボタンが離されたときのスナップ処理
                    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                        if (isDragging && draggingKeyFrame == &keyFrame) {
                            // 強制的にスナップ処理を適用
                            if (showSnapping) {
                                // 小数点以下の精度問題を回避するために一度整数に変換してから計算
                                double timeValue = keyFrame.time;
                                double intervalValue = snappingInterval;
                                int gridIndex = (int)round(timeValue / intervalValue);
                                keyFrame.time = gridIndex * snappingInterval;
                            }

                            // ドラッグ状態をリセット
                            isDragging = false;
                            draggingKeyFrame = nullptr;
                        }
                    }

                    // 右クリックメニュー（別の判定）
                    ImGui::SetCursorPos(ImVec2(keyX - kTimeMarkerSize, keyY - kTimeMarkerSize));
                    ImGui::InvisibleButton("keyframe_rmb", ImVec2(kTimeMarkerSize * 2.5f, kTimeMarkerSize * 2.5f),
                        ImGuiButtonFlags_MouseButtonRight);

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("KeyContextMenu");
                        keyFrame.isSelect = true;
                    }

                    if (ImGui::BeginPopup("KeyContextMenu")) {
                        if (ImGui::MenuItem("Delete")) {
                            // キーフレームの削除フラグを立てる
                            //sequenceEvent->MarkKeyframeForDeletion(keyFrame.time);
                        }
                        if (ImGui::MenuItem("Edit Value")) {
                            // 値編集ダイアログを開く
                        }
                        if (ImGui::BeginMenu("Easing")) {
                            static const char* easingTypes[] = {
                                "Linear", "EaseIn", "EaseOut", "EaseInOut"
                            };
                            for (int i = 0; i < 4; i++) {
                                if (ImGui::MenuItem(easingTypes[i])) {
                                    keyFrame.easingType = i;
                                }
                            }
                            ImGui::EndMenu();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::PopID();
                }
                // ドラッグ中のキーフレームが範囲外のときはドラッグを解除
                else if (isDragging && draggingKeyFrame == &keyFrame) {
                    // 強制的にスナップ処理を適用
                    if (showSnapping) {
                        // 小数点以下の精度問題を回避するために一度整数に変換してから計算
                        double timeValue = keyFrame.time;
                        double intervalValue = snappingInterval;
                        int gridIndex = (int)round(timeValue / intervalValue);
                        keyFrame.time = gridIndex * snappingInterval;
                    }

                    // ドラッグ状態をリセット
                    isDragging = false;
                    draggingKeyFrame = nullptr;

                }
            }

            trackY += kTrackHeight;
        }

        ImGui::EndChild();
    }

    // ===== ステータスバー =====
    {
        float statusBarY = windowPos.y + windowSize.y - kStatusBarHeight;
        ImVec2 statusBarPos = ImVec2(windowPos.x, statusBarY);
        ImVec2 statusBarSize = ImVec2(windowSize.x, kStatusBarHeight);

        drawList->AddRectFilled(
            statusBarPos,
            ImVec2(statusBarPos.x + statusBarSize.x, statusBarPos.y + statusBarSize.y),
            IM_COL32(40, 40, 40, 255)
        );

        char statusText[128];
        snprintf(statusText, sizeof(statusText), "Time: %.2fs | Scale: %.0f | Tracks: %zu",
            currentTime, timeScale, _sequence->GetSequenceEvents().size());

        drawList->AddText(
            ImVec2(statusBarPos.x + 5, statusBarPos.y + 2),
            IM_COL32(200, 200, 200, 255),
            statusText
        );
    }

    // 新しいイベント作成ボタン
    if (ImGui::Button("New Event")) {
        ImGui::OpenPopup("NewEventPopup");
    }

    if (ImGui::BeginPopup("NewEventPopup")) {
        static char newEventName[64] = "";
        static int selectedType = 0;
        static const char* eventTypes[] = { "float", "Vector2", "Vector3", "Vector4" };

        ImGui::InputText("Event Name", newEventName, sizeof(newEventName));
        ImGui::Combo("Type", &selectedType, eventTypes, IM_ARRAYSIZE(eventTypes));

        if (ImGui::Button("Create") && strlen(newEventName) > 0) {
            // 新しいイベントの作成
            switch (selectedType) {
            case 0: // float
                _sequence->CreateSequenceEvent<float>(newEventName, 0.0f);
                break;
            case 1: // Vector2
                _sequence->CreateSequenceEvent<Vector2>(newEventName, Vector2(0, 0));
                break;
            case 2: // Vector3
                _sequence->CreateSequenceEvent<Vector3>(newEventName, Vector3(0, 0, 0));
                break;
            case 3: // Vector4
                _sequence->CreateSequenceEvent<Vector4>(newEventName, Vector4(0, 0, 0, 0));
                break;
            }
            newEventName[0] = '\0';
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
    ImGui::PopID();
}


