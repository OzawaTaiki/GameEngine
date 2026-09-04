#include <Debug/GameViewportWindow.h>

#ifdef _DEBUG

#include <Core/WinApp/WinApp.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/RTV/RTVManager.h>
#include <System/Input/Input.h>

#include <imgui.h>

#endif // _DEBUG


namespace Engine {

GameViewportWindow* GameViewportWindow::GetInstance()
{
    static GameViewportWindow instance;
    return &instance;
}

void GameViewportWindow::Initialize([[maybe_unused]] const std::string& _renderTargetName)
{
#ifdef _DEBUG

    renderTargetName_ = _renderTargetName;

    rtvManager_ = RTVManager::GetInstance();
    dxCommon_ = DXCommon::GetInstance();
    input_ = Input::GetInstance();

#endif // _DEBUG
}

void GameViewportWindow::Show()
{
#ifdef _DEBUG

    if (input_->IsKeyTriggered(DIK_F2))
        isEnabled_ = !isEnabled_;

    // 無効時は全画面描画のまま。マウス座標の補正も外す
    if (!isEnabled_)
    {
        input_->ResetViewportRect();
        imagePos_ = { 0.0f,0.0f };
        imageSize_ = WinApp::kWindowSize_;
        return;
    }

    // 全画面に描かれたゲーム画面を消してエディタの背景にする
    ClearBackBuffer();
    // ImGuiがテクスチャとして読めるようにする
    TransitionRenderTargetToSRV();

    RenderTarget* renderTarget = rtvManager_->GetRenderTexture(renderTargetName_);

    ImGui::SetNextWindowSize(ImVec2(640.0f, 380.0f), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    const bool isOpened = ImGui::Begin("Game", &isEnabled_,
                                       ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PopStyleVar();

    if (isOpened && renderTarget)
    {
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const Vector2 fitSize = CalcFitSize(Vector2{ avail.x, avail.y });

        // レターボックス分だけ中央に寄せる
        const ImVec2 cursor = ImGui::GetCursorPos();
        ImGui::SetCursorPos(ImVec2(cursor.x + (avail.x - fitSize.x) * 0.5f,
                                   cursor.y + (avail.y - fitSize.y) * 0.5f));

        // ImGuiのスクリーン座標をウィンドウのクライアント座標に直す
        const ImVec2 screenPos = ImGui::GetCursorScreenPos();
        const ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
        imagePos_ = Vector2{ screenPos.x - viewportPos.x, screenPos.y - viewportPos.y };
        imageSize_ = fitSize;

        ImGui::Image(static_cast<ImTextureID>(renderTarget->GetGPUHandleofRTV().ptr),
                     ImVec2(fitSize.x, fitSize.y));

        if (correctMousePosition_)
            input_->SetViewportRect(imagePos_, imageSize_);
        else
            input_->ResetViewportRect();
    }
    else
    {
        // 折りたたみ中などはゲーム画面が見えないので補正も外す
        input_->ResetViewportRect();
        imageSize_ = { 0.0f,0.0f };
    }

    ImGui::End();

#endif // _DEBUG
}

void GameViewportWindow::TransitionRenderTargetToSRV()
{
#ifdef _DEBUG

    RenderTarget* renderTarget = rtvManager_->GetRenderTexture(renderTargetName_);
    if (!renderTarget)
        return;

    // RenderTarget::Draw() が描画後に RENDER_TARGET へ戻すため，ここで読める状態にする
    renderTarget->ChangeRTVState(dxCommon_->GetCommandList(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

#endif // _DEBUG
}

void GameViewportWindow::ClearBackBuffer()
{
#ifdef _DEBUG

    rtvManager_->ClearSwapChainRenderTexture(dxCommon_->GetSwapChain(),
                                             Vector4(backGroundColor_[0], backGroundColor_[1],
                                                     backGroundColor_[2], backGroundColor_[3]));

#endif // _DEBUG
}

Vector2 GameViewportWindow::CalcFitSize([[maybe_unused]] const Vector2& _avail) const
{
    Vector2 result = { 0.0f,0.0f };

#ifdef _DEBUG

    if (_avail.x <= 0.0f || _avail.y <= 0.0f)
        return result;

    const float aspect = WinApp::kWindowSize_.x / WinApp::kWindowSize_.y;

    result = _avail;
    if (_avail.x / _avail.y > aspect)
        result.x = _avail.y * aspect;    // 横が余る
    else
        result.y = _avail.x / aspect;    // 縦が余る

#endif // _DEBUG

    return result;
}

} // namespace Engine
