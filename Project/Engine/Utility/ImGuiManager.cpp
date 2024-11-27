#include "ImGuiManager.h"

#ifdef _DEBUG
#include "WinApp.h"
#include "DXCommon.h"
#include "SRVManager.h"

#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>
#endif // _DEBUG

void ImGuiManager::Initialize()
{
#ifdef _DEBUG

    srvManager_ = SRVManager::GetInstance();
    DXCommon* dx = DXCommon::GetInstance();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());

    if (!srvManager_)
        assert(false);

    srvManager_->Allocate();
    ImGui_ImplDX12_Init(
        dx->GetDevice(),
        static_cast<int>(dx->GetBackBufferSize()),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvManager_->GetSRVHeap_(),
        srvManager_->GetSRVHeap_()->GetCPUDescriptorHandleForHeapStart(),
        srvManager_->GetSRVHeap_()->GetGPUDescriptorHandleForHeapStart()
    );

#endif // _DEBUG
}

void ImGuiManager::Begin()
{
#ifdef _DEBUG
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Engine");
#endif // _DEBUG
}

void ImGuiManager::End()
{
#ifdef _DEBUG
    ImGui::End();
    ImGui::Render();
#endif // _DEBUG
}

void ImGuiManager::Draw()
{
#ifdef _DEBUG
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    ID3D12DescriptorHeap* ppheaps[] = { srvManager_->GetSRVHeap_() };
    commandList->SetDescriptorHeaps(_countof(ppheaps), ppheaps);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif // _DEBUG

}

void ImGuiManager::Finalize()
{
#ifdef _DEBUG
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
#endif // _DEBUG
}
