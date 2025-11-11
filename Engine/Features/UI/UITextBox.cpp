#include "UITextBox.h"

#include <System/Input/TextInputManager.h>
#include <Utility/ConvertString/ConvertString.h>
#include <System/Input/Input.h>
#include <System/Time/Time.h>

namespace
{
Vector2 padding = { 10.0f, 3.0f };
}

void UITextBox::Initialize(const std::string& _label, bool _regsterDebugWindow)
{
    UISelectable::Initialize(_label, _regsterDebugWindow);
    InitializeTextGenerator(FontConfig());
    textParam_.pivot.x = 0.0f;

    anchor_.x = 0.1f;
    anchor_.y = 0.5f;
    cursorTimer_ = 0.0f;
    cursorBlinkInterval_ = 0.5f;
    textOffset_ ={ 0.0f, 0.0f };

    textRect_.size = GetSize();
    Vector2 local = GetAnchor();
    textRect_.leftTop = GetLeftTopPos() - textRect_.size * local;
}

void UITextBox::UpdateSelf()
{
    if (!isActive_ || !isVisible_)
    {
        return;
    }

    UISelectable::UpdateSelf();

    if (!isAcceptingInput_)
        return;

    // テキスト入力の取得
    std::string inputText = TextInputManager::GetInstance()->GetInputText();
    if (!inputText.empty())
    {
        std::wstring wideInput = ConvertString(inputText);
        text_.insert(cursor_, wideInput);  // カーソル位置に挿入
        cursor_ += wideInput.length();     // カーソルを進める
    }

    // 入力処理
    ProcessInput();
    // カーソルの点滅更新
    UpdateCursor();

    textRect_.size = GetSize() - padding * 2;
    textRect_.leftTop = -size_ * anchor_ + padding;

}

void UITextBox::Draw()
{
    if (!isVisible_)
        return;

    UIInteractive::DrawSelf();

    Rect scrolledRect = textRect_;
    scrolledRect.leftTop.x -= textOffset_.x + padding.x;  // 文字列を左にスクロールする場合、rectを右にずらす

    // textOffset_ は pos ではなく rect に適用
    textGenerator_.Draw(text_, scrolledRect, GetWorldPos() + textOffset_ + padding, anchor_, { 1,1,1,1 }, GetOrder());
    // カーソルの描画
    DrawCursor();
    // | を挿入すると文字の位置がずれるため、別描画する
}

void UITextBox::OnClick()
{
    auto textInput = TextInputManager::GetInstance();
    textInput->Clear();
    textInput->SetAcceptingInput(true);
    isAcceptingInput_ = true;
}

void UITextBox::OnFocusGained()
{
    auto textInput = TextInputManager::GetInstance();
    textInput->Clear();
    textInput->SetAcceptingInput(true);
    isAcceptingInput_ = true;
}

void UITextBox::OnFocusLost()
{
    auto textInput = TextInputManager::GetInstance();
    textInput->SetAcceptingInput(false);
    isAcceptingInput_ = false;
    showCursor_ = false;
}

std::string UITextBox::GetText() const
{
    return ConvertString(text_);
}

void UITextBox::SetText(const std::string& _text)
{
    text_ = ConvertString(_text);
    cursor_ = text_.length();
}

void UITextBox::ImGuiContent()
{
#ifdef _DEBUG
    UISelectable::ImGuiContent();
    ImGui::Separator();
    textParam_.ImGui();
    ImGui::DragFloat2("Text Offset", &textOffset_.x, 1.0f);
    ImGui::Separator();
    ImGui::Text("Text Box Content: %s", ConvertString(text_).c_str());
    ImGui::Text("Cursor Position: %zu", cursor_);
#endif // _DEBUG

}

void UITextBox::DrawCursor()
{
    if (!showCursor_)
        return;

    const AtlasData* atlas = textGenerator_.GetAtlasData();
    if (atlas)
    {
        // カーソル位置のオフセットを計算
        float cursorXOffset = CalculateCursorXPosition();

        // テキスト全体のサイズを取得（pivot補正用）
        Vector2 stringArea = atlas->GetStringAreaSize(text_, textParam_.scale);

        // テキスト開始位置（pivotを考慮）
        Vector2 textStartPos = GetWorldPos();
        textStartPos.x -= stringArea.x * textParam_.pivot.x;
        textStartPos.y -= stringArea.y * textParam_.pivot.y;

        // カーソルの絶対座標（スクロールオフセットを適用）
        Vector2 cursorPos = textStartPos;
        cursorPos.x += cursorXOffset + textOffset_.x;
        cursorPos.y += stringArea.y * 0.5f; // 中央揃え

        // カーソルを描画
        TextParam cursorParam = textParam_;
        cursorParam.position = cursorPos;
        cursorParam.pivot.x = 0.5f;  // 中央揃え

        textGenerator_.Draw(L"|", cursorParam, GetOrder() + 1);
    }

}

void UITextBox::ProcessInput()
{
    // 更新ですでに確認しているため フラグチェックは省略

    auto input = Input::GetInstance();

    // バックスペース、デリート処理
    if (input->IsKeyTriggered(DIK_BACKSPACE))
    {
        BackSpace();
    }
    if (input->IsKeyTriggered(DIK_DELETE))
    {
        Delete();
    }

    // カーソル移動
    MoveCursor();

    // スクロールオフセット更新
    UpdateScrollOffset();

    // Enterキーでフォーカスを外す
    if (input->IsKeyTriggered(DIK_RETURN))
    {
        OnFocusLost();
    }

}

void UITextBox::BackSpace()
{
    size_t len = text_.length();
    if (len == 0 || cursor_ == 0)
    {
        return;
    }
    text_.erase(cursor_ - 1, 1);
    if (cursor_ > 0)
        cursor_--;
}

void UITextBox::Delete()
{
    size_t len = text_.length();
    if (len == 0 || cursor_ >= len)
    {
        return;
    }
    text_.erase(cursor_, 1);
}

void UITextBox::MoveCursor()
{
    auto input = Input::GetInstance();

    if (input->IsKeyTriggered(DIK_LEFT))
    {
        if (cursor_ > 0)
            cursor_--;
    }
    else if (input->IsKeyTriggered(DIK_RIGHT))
    {
        if (cursor_ < text_.length())
            cursor_++;
    }
    else if (input->IsKeyTriggered(DIK_UP))
    {
        cursor_ = 0;
    }
    else if (input->IsKeyTriggered(DIK_DOWN))
    {
        cursor_ = text_.length();
    }

    cursor_ = std::clamp(cursor_, size_t(0), text_.length());
}

void UITextBox::InitializeTextGenerator(const FontConfig& _fontConfig)
{
    textGenerator_.Initialize(_fontConfig);
}

void UITextBox::UpdateCursor()
{
    cursorTimer_ += Time::GetDeltaTime<float>();
    if (cursorTimer_ >= cursorBlinkInterval_)
    {
        // 点滅タイマーリセットと表示/非表示切り替え
        cursorTimer_ =std::fmod(cursorTimer_, cursorBlinkInterval_);
        showCursor_ = !showCursor_;
    }
}
float UITextBox::CalculateCursorXPosition() const
{
    // カーソルが先頭なら0
    if (cursor_ == 0)
        return padding.x;

    const AtlasData* atlas = textGenerator_.GetAtlasData();
    if (!atlas)
        return 0.0f;

    float currentX = 0.0f;
    float fontSize = atlas->GetFontSize();
    // TextRenderer::DrawText() と同じロジック
    for (size_t i = 0; i < cursor_ && i < text_.length(); ++i)
    {
        wchar_t character = text_[i];

        // 改行処理
        if (character == L'\n')
        {
            currentX = padding.x; // 行頭に戻る
            continue;
        }

        // スペース処理
        if (character == L' ')
        {
            currentX += fontSize * 0.3f;
            continue;
        }

        // グリフ取得
        GlyphInfo glyph = atlas->GetGlyph(character);
        if (!glyph.isValid)
            continue;

        // 次の文字位置 = カーソル位置（TextRenderer.cpp 203行目）
        currentX += glyph.advance;
    }

    return currentX * textParam_.scale.x + padding.x;
}

void UITextBox::UpdateScrollOffset()
{
    if (!isAcceptingInput_)
        return;

    // カーソルのローカルX座標（textOffset_適用前）
    float cursorX = CalculateCursorXPosition();

    // スクロール適用後の表示位置
    float displayCursorX = cursorX + textOffset_.x;

    // textRect_のサイズ
    float rectWidth = textRect_.size.x;
    float margin = padding.x;

    // カーソルが右端を超えた場合
    if (displayCursorX > rectWidth - margin)
    {
        textOffset_.x -= (displayCursorX - (rectWidth - margin));
    }

    // カーソルが左端より左にある場合
    if (displayCursorX < margin)
    {
        textOffset_.x += (margin - displayCursorX);
    }

    // スクロール範囲制限（右端は無制限、左端は0まで）
    if (textOffset_.x > 0.0f)
    {
        textOffset_.x = 0.0f;
    }
}
