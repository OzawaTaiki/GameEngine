#include "ImguITools.h"

#include <Debug/ImGuiHelper.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

#include <numbers>
#include <list>

void ImGuiTool::DrawTitleBar(const char* _text, const ImVec4& _color)
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

void ImGuiTool::DrawGradientEditor(std::list<std::pair<float, Vector4>>& _colors)
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

    drawlist->AddImage(tex, rectPos, ImVec2(rectPos.x + rectSize.x, rectPos.y + rectSize.y),ImVec2(0,0), ImVec2(1,1),ImU32(0xffffff80));

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

        if (ImGui::IsItemActive())            isActive = true;
        if (ImGui::IsItemHovered())            isHover = true;


        ImGui::SetCursorScreenPos(botButtonPos);
        ImGui::InvisibleButton(botLabel.c_str(), pentagonSize);

        if (ImGui::IsItemActive())            isActive = true;
        if (ImGui::IsItemHovered())            isHover = true;

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
            if(ImGui::Button("delete"))
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

    // 変更がないとき
    if (!hasChanged)
    {
        // 新しく要素を追加できる
        if (ImGui::IsMouseClicked(1))
        {
            // マウスの座標を取得
            ImVec2 mousePos = ImGui::GetMousePos();
            // 0 - 1で正規化
            float time = (mousePos.x - cursorPos.x) / rectSize.x;
            // クランプ
            if (time < 0)       time = 0;
            if (time > 1)       time = 1;
            // 色を追加
            _colors.push_back({ time, Vector4(1,1,1,1) });
            hasChanged = true;
        }

    }

    // 値の変更があったときだけソート
    if (hasChanged)
    {
        // colorsをtimeでソート
        _colors.sort([](const std::pair<float, Vector4>& a, const std::pair<float, Vector4>& b) {return a.first < b.first; });

        // 0と1を設定
        _colors.begin()->first = 0;
        _colors.rbegin()->first = 1;
    }

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

void ImGuiTool::GradientEditor(const char* _label, std::list<std::pair<float, Vector4>>& _colors)
{

    ImGui::PushID(_label);


    if (ImGui::Button("Open Editor"))
    {
        ImGui::OpenPopup("GradientEditor");

        // colorsをtimeでソート
        _colors.sort([](const std::pair<float, Vector4>& a, const std::pair<float, Vector4>& b) {return a.first < b.first; });

    }
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    if (ImGui::BeginPopup("GradientEditor"))
    {
        ImGuiTool::DrawTitleBar("GradientEditor", ImVec4(1, 1, 1, 1));

        DrawGradientEditor(_colors);

        ImGui::EndPopup();
    }


    ImGui::PopID();

}


