#pragma once

#include <Features/TextRenderer/TextGenerator.h>
#include <Features/UI/UISelectable.h>
#include <Math/Rect/Rect.h>

/// <summary>
/// テキストボックスUI
/// 背景なし、テキスト入力可能なUI
/// 単一行のみ対応
/// </summary>
class UITextBox : public UISelectable
{
public:

    UITextBox() = default;
    ~UITextBox() override = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="_label">UI識別用ラベル</param>
    /// <param name="_regsterDebugWindow">デバッグウィンドウ登録フラグ</param>
    void Initialize(const std::string& _label, bool _regsterDebugWindow = true) override;

    /// <summary>
    /// 更新
    /// </summary>
    void UpdateSelf() override;

    /// <summary>
    /// 描画
    /// </summary>
    void Draw() override;

    /// <summary>
    /// テキスト表示領域の設定
    /// </summary>
    /// <param name="_rect">表示領域</param>
    void SetTextRect(const Rect& _rect) { textRect_ = _rect; }

    /// <summary>
    /// フォント設定の設定
    /// </summary>
    /// <param name="_fontConfig">フォント設定</param>
    void SetFontConfig(const FontConfig& _fontConfig) { InitializeTextGenerator(_fontConfig); }

    void SetTextParam(const TextParam& _param) { textParam_ = _param; }
    TextParam& GetTextParam() { return textParam_; }

    void OnClick() override;

    void OnFocusGained() override;

    void OnFocusLost() override;

    std::string GetText() const;

private:

    /// <summary>
    /// カーソルの点滅更新
    /// </summary>
    void UpdateCursor();
    /// <summary>
    /// カーソル描画
    /// </summary>
    void DrawCursor();

    /// <summary>
    /// 入力処理
    /// </summary>
    void ProcessInput();

    /// <summary>
    /// バックスペース処理
    /// </summary>
    void BackSpace();
    /// <summary>
    /// デリート処理
    /// </summary>
    void Delete();
    /// <summary>
    /// カーソル移動処理
    /// </summary>
    void MoveCursor();
    /// <summary>
    /// テキストジェネレータ初期化
    /// </summary>
    /// <param name="_fontConfig">フォント設定</param>
    void InitializeTextGenerator(const FontConfig& _fontConfig);
    /// <summary>
    /// カーソルのX座標計算
    /// </summary>
    /// <returns>カーソルのX座標</returns>
    float CalculateCursorXPosition() const;
private:

    bool isAcceptingInput_= false;

    Rect textRect_;

    std::wstring text_;

    size_t cursor_ = 0;

    TextGenerator textGenerator_;

    TextParam textParam_;

    float cursorTimer_ = 0.0f; // カーソル点滅タイマー
    float cursorBlinkInterval_ = 0.5f; // カーソル点滅間隔（秒）
    bool showCursor_ = true; // カーソル表示フラグ

};