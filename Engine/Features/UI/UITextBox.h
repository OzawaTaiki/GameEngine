#pragma once

#include <Features/TextRenderer/TextGenerator.h>
#include <Features/UI/UISelectable.h>
#include <Math/Rect/Rect.h>

enum class InputTextType
{
    Text,       // 全ての文字
    Integer,    // 整数のみ
    Float,      // 浮動小数点数
    FileName,   // ファイル名に使えない文字以外
    Alphabet,   // 英字のみ
    Numeric     // 数字のみ
};

/// <summary>
/// テキストボックスUI
/// 背景なし、テキスト入力可能なUI
/// 単一行のみ対応
/// </summary>
class UITextBox : public UISelectable
{
public:

    UITextBox(InputTextType _inputTextType = InputTextType::Text)
        : inputTextType_(_inputTextType){ }
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
    /// テキストを任意の型で取得
    /// </summary>
    /// <typeparam name="T">期待する型型</typeparam>
    /// <param name="outValue">出力先変数</param>
    /// <returns>変換成功ならtrue</returns>
    template<typename T>
    bool GetValueAs(T& outValue) const
    {
        std::istringstream iss(confirmedText_);
        iss >> outValue;
        return !iss.fail() && iss.eof();  // 変換成功 & 全文字列消費
    }

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

    // ==================
    // コールバック
    // ==================
    void OnClick() override;

    void OnFocusGained() override;

    void OnFocusLost() override;

    void SetOnConfirmed(const std::function<void()>& _callback) { onTextConfirmed_ = _callback; }


    /// <summary>
    /// 確定テキストの取得
    /// </summary>
    /// <returns>確定テキスト</returns>
    std::string GetConfirmedText() const { return confirmedText_; }

    /// <summary>
    /// テキストの設定
    /// </summary>
    /// <param name="_text">設定テキスト</param>
    void SetText(const std::string& _text);
    /// 初期化時などに使用する

    template<typename T>
    void SetValue(T _value);

    void ImGuiContent() override;

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
    /// <summary>
    /// スクロールオフセット更新
    /// </summary>
    void UpdateScrollOffset();

    /// <summary>
    /// 入力文字の検証
    /// </summary>
    /// <returns>>有効な文字ならtrue</returns>
    bool ValidateInputChar();

    // ヘルパー
    std::string GetInputTextTypeString() const;


private:

    bool isAcceptingInput_= false;

    Rect textRect_;

    // 確定前のテキスト
    std::string pendingText_;
    // 確定テキスト
    std::string confirmedText_;

    InputTextType inputTextType_ = InputTextType::Text;

    size_t cursor_ = 0;

    TextGenerator textGenerator_;

    TextParam textParam_;
    Vector2 textOffset_;

    float cursorTimer_ = 0.0f; // カーソル点滅タイマー
    float cursorBlinkInterval_ = 0.5f; // カーソル点滅間隔（秒）
    bool showCursor_ = true; // カーソル表示フラグ


    std::function<void()> onTextConfirmed_; // テキスト確定時コールバック
};

template<typename T>
inline void UITextBox::SetValue(T _value)
{
    std::string formattedText;

    switch (inputTextType_)
    {
        case InputTextType::Integer:
        case InputTextType::Numeric:
        {
            // 整数としてフォーマット
            formattedText = std::to_string(static_cast<int>(_value));
            break;
        }
        case InputTextType::Float:
        {
            // 浮動小数点数としてフォーマット
            std::ostringstream oss;
            oss << _value;
            formattedText = oss.str();
            break;
        }
        default:
        {
            // その他の型は文字列化
            formattedText = std::to_string(_value);
            break;
        }
    }

    pendingText_ = formattedText;
    confirmedText_ = formattedText;
    cursor_ = pendingText_.length();
}
