#include "ImGuiManager.h"
#include "WinApp.h"
#include "DXCommon.h"
#include "SRVManager.h"

#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

//extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ImGuiManager::Initialize(SRVManager* _ptr)
{
    srvManager_ = _ptr;
    DXCommon* dx = DXCommon::GetInstance();

    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(WinApp::GetInstance()->GetHwnd());


    srvManager_->Allocate();
    ImGui_ImplDX12_Init(
        dx->GetDevice(),
        static_cast<int>(dx->GetBackBufferSize()),
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvManager_->GetSRVHeap_(),
        srvManager_->GetSRVHeap_()->GetCPUDescriptorHandleForHeapStart(),
        srvManager_->GetSRVHeap_()->GetGPUDescriptorHandleForHeapStart()
    );



}

void ImGuiManager::Begin()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::End()
{
    ImGui::Render();
}

void ImGuiManager::Draw()
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();
    ID3D12DescriptorHeap* ppheaps[] = { srvManager_->GetSRVHeap_() };
    commandList->SetDescriptorHeaps(_countof(ppheaps), ppheaps);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

}

void ImGuiManager::Finalize()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
