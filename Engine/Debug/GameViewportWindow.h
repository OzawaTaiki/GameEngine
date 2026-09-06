#pragma once

#include <Math/Vector/Vector2.h>

#include <string>


namespace Engine {

class RTVManager;
class DXCommon;
class Input;

/// <summary>
/// Unityのようにゲーム画面をImGuiウィンドウ内に表示するデバッグ用ビューポート
/// 有効な間はスワップチェインへの全画面描画を塗りつぶし，
/// マウス座標をビューポート内のゲーム座標へ補正する
/// </summary>
class GameViewportWindow
{
public:

    static GameViewportWindow* GetInstance();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="_renderTargetName">表示するRenderTargetの名前</param>
    void Initialize(const std::string& _renderTargetName = "default");

    /// <summary>
    /// ビューポートウィンドウを構築する
    /// スワップチェインをレンダーターゲットに設定した後，
    /// ImGuiManager::Draw() より前に呼ぶこと
    /// </summary>
    void Show();

    /// <summary> ビューポート表示モードかどうか </summary>
    bool IsEnabled() const { return isEnabled_; }
    void SetEnable(bool _enable) { isEnabled_ = _enable; }
    /// <summary> メニューのチェックボックス用 </summary>
    bool* GetEnablePtr() { return &isEnabled_; }

    /// <summary> ゲーム画面が描かれている矩形の左上(クライアント座標) </summary>
    const Vector2& GetImagePosition() const { return imagePos_; }
    /// <summary> ゲーム画面が描かれている矩形のサイズ(クライアント座標) </summary>
    const Vector2& GetImageSize() const { return imageSize_; }

private:

    /// <summary> 表示中のRenderTargetをシェーダリソースとして読める状態にする </summary>
    void TransitionRenderTargetToSRV();

    /// <summary> スワップチェインに描かれた全画面のゲーム画面を塗りつぶす </summary>
    void ClearBackBuffer();

    /// <summary> アスペクト比を維持した表示サイズを求める </summary>
    Vector2 CalcFitSize(const Vector2& _avail) const;

    std::string renderTargetName_ = "default";

    // ビューポート表示モードか
    bool isEnabled_ = true;
    // ウィンドウ自体が開かれているか
    bool isWindowOpen_ = true;
    // マウス座標の補正を行うか
    bool correctMousePosition_ = true;

    Vector2 imagePos_ = { 0.0f,0.0f };
    Vector2 imageSize_ = { 0.0f,0.0f };

    // ビューポートの余白の色
    float backGroundColor_[4] = { 0.10f,0.10f,0.11f,1.0f };

    RTVManager* rtvManager_ = nullptr;
    DXCommon* dxCommon_ = nullptr;
    Input* input_ = nullptr;

private:
    GameViewportWindow() = default;
    ~GameViewportWindow() = default;
    // コピー禁止
    GameViewportWindow(const GameViewportWindow&) = delete;
    GameViewportWindow& operator=(const GameViewportWindow&) = delete;

};

} // namespace Engine
