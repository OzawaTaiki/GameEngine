#include "ImGuiHelper.h"

#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Animation/Sequence/AnimationSequence.h>

#include <cmath>
#include <algorithm>
#include <numbers>

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


void ImGuiHelper::DrawGradientEditor(std::list<std::pair<float, Vector4>>& _colors)
{
    // draw_listの取得
    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    // 座標を取得
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    // 矩形のサイズを決める
    ImVec2 rectSize = ImVec2(256, 96);

    // 五角形のサイズを決める
    ImVec2 pentagonSize = ImVec2(10, 15);

    // 矩形の座標を決める
    ImVec2 rectPos = ImVec2(cursorPos.x, cursorPos.y + pentagonSize.y);

    // マウスの座標を取得
    ImVec2 mousePos = ImGui::GetMousePos();


    // 矩形背景画像を描画
    static uint32_t i = TextureManager::GetInstance()->Load("checker_monochrome.png");
    static auto tex = TextureManager::GetInstance()->GetGPUHandle(i).ptr;

    drawlist->AddImage(tex, rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y), ImVec2(0, 0), ImVec2(1, 1), ImU32(0xffffff80));

    // グラデーションを描画するためのリスト
    std::list<std::pair<float, ImU32>> gradientRects;

    uint32_t id = 0;
    bool isHover = false;
    bool isActive = false;
    bool hasChanged = false;


    // 基準となる座標を計算
    ImVec2 upBasePos = ImVec2(cursorPos.x, cursorPos.y + pentagonSize.y / 2.0f);
    ImVec2 botBasePos = ImVec2(cursorPos.x, rectPos.y + rectSize.y + pentagonSize.y / 2.0f);


    for (auto it = _colors.begin(); it != _colors.end();)
    {
        isActive = false;
        isHover = false;

        float& time = it->first;
        Vector4& color = it->second;

        ImVec4 colVec4 = ImVec4(color.x, color.y, color.z, color.w);
        ImU32 colU32 = ImGui::GetColorU32(colVec4);

        // 五角形の中心座標を計算
        ImVec2 upCenterPos = ImVec2(upBasePos.x + rectSize.x * time, upBasePos.y);
        ImVec2 botCenterPos = ImVec2(botBasePos.x + rectSize.x * time, botBasePos.y);

        // 判定用の座標を計算
        ImVec2 upButtonPos = ImVec2(upCenterPos.x - pentagonSize.x / 2.0f, upCenterPos.y - pentagonSize.y / 2.0f);
        ImVec2 botButtonPos = ImVec2(botCenterPos.x - pentagonSize.x / 2.0f, botCenterPos.y - pentagonSize.y / 2.0f);

        // ラベルを作成
        std::string upLabel = "##upPenta" + std::to_string(id++);
        std::string botLabel = "##botPenta" + std::to_string(id++);

        // 座標を設定
        ImGui::SetCursorScreenPos(upButtonPos);
        // 判定用のボタンを作成
        ImGui::InvisibleButton(upLabel.c_str(), pentagonSize);

        if (ImGui::IsItemActive())
            isActive = true;
        if (ImGui::IsItemHovered())
            isHover = true;


        ImGui::SetCursorScreenPos(botButtonPos);
        ImGui::InvisibleButton(botLabel.c_str(), pentagonSize);

        if (ImGui::IsItemActive())
            isActive = true;
        if (ImGui::IsItemHovered())
            isHover = true;

        // ホバー時とアクティブ時の色を設定
        ImU32 pentaColor;
        if (isHover || isActive)
        {
            pentaColor = ImU32(0xf08080ff);
            hasChanged = true;
        }
        else
        {
            pentaColor = ImU32(0xffffffff);
        }

        std::string popupLabel = "##ColorEdit" + std::to_string(id);
        if (isHover)
        {
            // マウスの左クリックが押されたとき
            if (ImGui::IsMouseClicked(1))
            {
                // pop upを表示
                ImGui::OpenPopup(popupLabel.c_str());
            }
        }

        if (ImGui::BeginPopup(popupLabel.c_str()))
        {
            if (ImGui::ColorEdit4("Color", &color.x))
            {
                // 色を変更
                colVec4 = ImVec4(color.x, color.y, color.z, color.w);
                colU32 = ImGui::GetColorU32(colVec4);
            }
            if (ImGui::DragFloat("Time", &time, 0.01f, 0.0f, 1.0f))
            {
                hasChanged = true;
            }

            if (ImGui::Button("delete"))
            {
                it = _colors.erase(it);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // スライダー機能
        if (isActive)
        {
            // 0 - 1で正規化
            time = (mousePos.x - cursorPos.x) / rectSize.x;

            // クランプ
            if (time < 0)       time = 0;
            if (time > 1)       time = 1;

            // 座標を再計算
            upCenterPos = ImVec2(upBasePos.x + rectSize.x * time, upBasePos.y);
            botCenterPos = ImVec2(botBasePos.x + rectSize.x * time, botBasePos.y);

            hasChanged = true;
        }

        // posの昇順になるように挿入する
        auto insertIt = std::find_if(gradientRects.begin(), gradientRects.end(), [&upCenterPos](const std::pair<float, ImU32>& a) {return a.first > upCenterPos.x; });
        gradientRects.insert(insertIt, { upCenterPos.x, colU32 });

        // 五角形を描画
        ImGuiHelper::DrawRightPentagon(upCenterPos, pentagonSize, 0, pentaColor);
        ImGuiHelper::DrawRightPentagon(botCenterPos, pentagonSize, std::numbers::pi_v<float>, pentaColor);

        ++it;
    }

    std::string addPopupLabel = "##ColorAdd" + std::to_string(id);
    // 変更がないとき
    if (!hasChanged)
    {
        if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup(addPopupLabel.c_str());
        }
    }

    if (ImGui::BeginPopup(addPopupLabel.c_str()))
    {
        Vector4 color = Vector4(1, 1, 1, 1);
        //    // 0 - 1で正規化
        float time = 0.5f;
        if (time < 0) time = 0;
        if (time > 1) time = 1;

        if (ImGui::ColorEdit4("Color", &color.x))
        {
            // 色を変更
            ImVec4 colVec4 = ImVec4(color.x, color.y, color.z, color.w);
        }
        if (ImGui::DragFloat("Time", &time, 0.01f, 0.0f, 1.0f))
        {
            hasChanged = true;
        }
        if (ImGui::Button("add"))
        {
            _colors.push_back({ time, Vector4(1,1,1,1) });
            hasChanged = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }


    // 値の変更があったときだけソート
    if (ImGui::GetIO().MouseReleased[0])
    {
        // colorsをtimeでソート
        _colors.sort([](const std::pair<float, Vector4>& a, const std::pair<float, Vector4>& b) {return a.first < b.first; });

        // 0と1を設定
        _colors.begin()->first = 0;
        _colors.rbegin()->first = 1;
    }

    ImGui::BeginListBox("##ColorList");
    for (auto it = _colors.begin(); it != _colors.end(); ++it)
    {
        ImGui::PushID(&(*it));
        ImGui::Text("Time:%f", it->first);
        ImGui::SameLine();
        ImGui::ColorButton("Color", ImVec4(it->second.x, it->second.y, it->second.z, it->second.w));
        ImGui::PopID();
    }
    ImGui::EndListBox();

    // グラデーションを描画
    for (auto it = gradientRects.begin(); it != gradientRects.end(); ++it)
    {
        // 次の要素を取得
        auto next = std::next(it);
        // 次の要素がある場合
        if (next != gradientRects.end())
        {
            // 座標を計算
            ImVec2 nextPos = ImVec2(next->first, rectPos.y);
            ImVec2 pos = ImVec2(it->first, rectPos.y + rectSize.y);
            // 描画
            drawlist->AddRectFilledMultiColor(pos, nextPos, it->second, next->second, next->second, it->second);
        }
    }
}

void ImGuiHelper::TimeLine::Draw(AnimationSequence* _sequence)
{
    ImGui::Begin("TimeLine");

    if (_sequence)
    {
        static float currentTime = 0;

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        //static float currentTime = 0;

        //ImGui::Text("TimeLine");
        // シーケンスの時間を取得

        auto sequenceEvents = _sequence->GetSequenceEvents();
        static ImVec4 defaultColor = ImVec4(1.0f, 1.0f, 1.0f, 0.086f);
        static ImVec4 hoverColor = ImVec4(1.0f, 1.0f, 1.0f, 0.290f);

        //ImGui::ColorEdit4("defaultColor", &defaultColor.x);
        //ImGui::ColorEdit4("hoverColor", &hoverColor.x);

        // 時間軸の描画
        float rectPosXOffset = 100.0f;
        ImVec2 basePos = ImGui::GetCursorScreenPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImVec2 timeAxisRectSize = ImVec2(windowSize.x - 15, 20);
        ImVec2 timeAxisRectPos = basePos;
        ImVec2 timeAxisCountStartPos = timeAxisRectPos;
        timeAxisCountStartPos.x += 100;
        ImVec2 mousePos = ImGui::GetMousePos();
        float rangePerSec = 100.0f;

        // 時間軸の背景を描画
        drawList->AddRectFilled(timeAxisCountStartPos, ImVec2(timeAxisCountStartPos.x + timeAxisRectSize.x, timeAxisCountStartPos.y + timeAxisRectSize.y), ImU32(0x30ffffff));

        // 時間軸のメモリを描画
        for (int i = 0; i < 10; i++)
        {
            ImVec2 lineStartPos = ImVec2(timeAxisCountStartPos.x + rectPosXOffset + i * rectPosXOffset, timeAxisCountStartPos.y);
            ImVec2 lineEndPos = ImVec2(timeAxisCountStartPos.x + rectPosXOffset + i * rectPosXOffset, timeAxisCountStartPos.y + timeAxisRectSize.y);
            drawList->AddLine(lineStartPos, lineEndPos, ImU32(0x30ffffff));
        }

        ImVec2 timeZerpPoint = timeAxisCountStartPos;
        ImVec2 pentagonSize = ImVec2(11, 15);
        ImVec2 pentagonPos = timeZerpPoint;
        pentagonPos.x = timeZerpPoint.x + rangePerSec * currentTime;
        pentagonPos.y += pentagonSize.y / 2 + 2;

        ImU32 pentagonColorU32 = ImU32(0xb0ffffff);

        ImVec2 pentagonInvisibleButtonPos = pentagonPos;
        //pentagonInvisibleButtonPos.x
        ImGui::SetCursorScreenPos(ImVec2(pentagonPos.x - 15/ 2, timeAxisCountStartPos.y));
        ImGui::InvisibleButton("##TimeAxisPentagon", ImVec2(15, 15));

        if (ImGui::IsItemHovered())
            pentagonColorU32 = ImU32(0xff0000ff);
        if (ImGui::IsItemActive())
        {
            pentagonColorU32 = ImU32(0xff0000ff);
            currentTime = (mousePos.x - timeAxisCountStartPos.x) / rangePerSec;
            currentTime = currentTime < 0 ? 0 : currentTime;
            pentagonPos.x = timeZerpPoint.x + rangePerSec * currentTime;
        }

        DrawRightPentagon(pentagonPos, pentagonSize, 0.0f, pentagonColorU32);
        drawList->AddLine(pentagonPos, ImVec2(pentagonPos.x, pentagonPos.y + windowSize.y), pentagonColorU32);




        //ImGui::Dummy(timeAxisRectSize);
        ImVec2 OverlapRectPos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos(ImVec2(OverlapRectPos.x, OverlapRectPos.y + 5));
        OverlapRectPos = ImGui::GetCursorScreenPos();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));          // 背景を透明に
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 0, 0));   // ホバー時の背景も透明に
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));    // クリック時の背景も透明に
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);            // 枠線をなくす
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        ImGui::Text("OverlapRect");
        ImGui::SetCursorScreenPos(OverlapRectPos);
        if (ImGui::Button("##OverlapRect", ImVec2(100, 15)))
        {
            if (1)
            {

            }
        }

        std::string TreeNodeLabel = _sequence->GetLabel();
        if (ImGui::TreeNode(TreeNodeLabel.c_str()))
        {


            for (auto& sequenceEvent : sequenceEvents)
            {
                std::string l = sequenceEvent->GetLabel();
                std::list<SequenceEvent::KeyFrame>& keyFrames = sequenceEvent->GetKeyFrames();
                ImGui::PushID(&sequenceEvent);
                if (ImGui::IsMouseClicked(0))
                {
                    sequenceEvent->SetSelect(false);
                }

                // ラベルとTimeLine枠を表示
                ImVec2 rectMinPos = ImGui::GetCursorScreenPos();
                rectMinPos.x += rectPosXOffset;

                ImVec2 rectSize = ImGui::GetWindowSize();
                rectSize.y = 20;
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImGui::Text(l.c_str());
                ImGui::SetCursorScreenPos(cursorPos);


                ImU32 color = ImGui::GetColorU32(defaultColor);
                ImGui::InvisibleButton(sequenceEvent->GetLabel().c_str(), rectSize);
                if (ImGui::IsItemHovered())
                {
                    color = ImGui::GetColorU32(hoverColor);
                    if (ImGui::IsMouseClicked(0))
                    {
                        sequenceEvent->SetSelect(true);
                    }
                }

                if (sequenceEvent->IsSelect())
                {
                    color = ImGui::GetColorU32(hoverColor);
                }

                ImGui::SameLine();
                rectSize.x -= rectPosXOffset + 33;
                ImVec2 rectMaxPos = ImVec2(rectMinPos.x + rectSize.x, rectMinPos.y + rectSize.y);
                drawList->AddRectFilled(rectMinPos, rectMaxPos, color);
                ImGui::Dummy(rectSize);
                //drawList->AddRectFilled(ImVec2(rectMinPos.x + 100, rectMinPos.y), ImVec2(100, 100), color);


                ImVec2 zeroPoint = rectMinPos;
                int32_t id = 0;
                for (auto& keyFrame : keyFrames)
                {
                    ImGui::Text("Time:%f", keyFrame.time);
                    ImGui::SameLine();
                    ImGui::Text("Value:%f", std::get<float>(keyFrame.value));

                    float time = keyFrame.time;

                    float rectLocalPos = rangePerSec * time;

                    ImVec2 markPosition = zeroPoint;
                    markPosition.x += rectLocalPos;
                    markPosition.y += 10;
                    ImU32 markColor = ImU32(0xffffffff);

                    std::string invisibleButtonLabel = "##TimeLine" + l + std::to_string(id);
                    ImGui::SetCursorScreenPos(ImVec2(markPosition.x - 7, markPosition.y - 7));
                    ImGui::InvisibleButton(invisibleButtonLabel.c_str(), ImVec2(16, 16));

                    // 重なってるとき赤く
                    if (ImGui::IsItemHovered())
                        markColor = ImU32(0xd00000ff);
                    if (ImGui::IsItemActive())
                    {
                        markColor = ImU32(0xd00000ff);
                        keyFrame.isSelect = true;
                    }
                    if (keyFrame.isSelect)
                    {
                        markColor = ImU32(0xd00000ff);
                        if (ImGui::IsMouseDown(0) && !ImGui::IsMouseClicked(0))
                        {

                            time = (mousePos.x - zeroPoint.x) / rangePerSec;
                            time = time < 0 ? 0 : time;

                            if (ImGui::IsMouseDown(1))
                            {
                                // 0,1刻み
                                time = std::round(time * 10.0f) / 10.0f;
                            }

                            keyFrame.time = time;

                            rectLocalPos = rangePerSec * time;
                            markPosition = zeroPoint;
                            markPosition.x += rectLocalPos;
                            markPosition.y += 10;
                        }
                    }

                    drawList->AddCircleFilled(markPosition, 7, markColor);
                    //sequenceEvent->ImDragValue();
                }
                ImGui::PopID();
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleVar(2);       // 変更したスタイルを元に戻す
        ImGui::PopStyleColor(3);    // 変更した3つの色を元に戻す
        drawList->AddRectFilled(ImVec2(timeAxisCountStartPos.x, OverlapRectPos.y),
            ImVec2(timeAxisCountStartPos.x + timeAxisRectSize.x, OverlapRectPos.y + windowSize.y),
            ImU32(0xa0202020));

        // シーケンスの時間を表示
        ImGui::Text("CurrentTime:%f", currentTime);

    }

    ImGui::End();

}


void ImGuiHelper::DrawTitleBar(const char* _text)
{
    // 描画リストの取得
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImU32 TitlebatColor = ImGui::GetColorU32(ImGuiCol_TabActive);
    ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);


    ImVec2 textSize = ImGui::CalcTextSize(_text);



    ImVec2 titleBarsize = textSize;
    titleBarsize.x = ImGui::GetWindowWidth();
    titleBarsize.y *= 2.0f;
    if (titleBarsize.x < 100)
        titleBarsize.x = 100;

    ImVec2 padding = ImVec2(10, textSize.y / 2.0f);

    ImVec2 pos = ImGui::GetWindowPos();

    // 背景矩形を描画
    //draw_list->AddRectFilled(pos, ImVec2(pos.x + titleBarsize.x, pos.y + titleBarsize.y), ImGui::GetColorU32(_color));
    draw_list->AddRectFilled(pos, ImVec2(pos.x + titleBarsize.x, pos.y + titleBarsize.y), TitlebatColor);

    // テキストを描画
    draw_list->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y), textColor, _text);

    ImGui::SetCursorPos(ImVec2(10, titleBarsize.y + 10));
}
