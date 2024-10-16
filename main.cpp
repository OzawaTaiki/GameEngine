#include "WinApp.h"
#include "DXCommon.h"
#include "myLib/MyLib.h"
#include "Debug.h"
#include "Input.h"
#include "TextureManager.h"
#include "WorldTransform.h"
#include "ModelManager.h"
#include "Camera.h"
#include "ObjectColor.h"
#include "Sprite.h"

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include <random>

const float kDeltaTime = 1.0f / 60.0f;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initilize();

	DXCommon* dxCommon =  DXCommon::GetInstance();
	dxCommon->Initialize(winApp,WinApp::kWindowWidth_, WinApp::kWindowHeight_);

	TextureManager::GetInstance()->Initialize();
	TextureManager::GetInstance()->Load("uvChecker.png");

	PSOManager::GetInstance()->Initialize();
	Sprite::StaticInitialize(WinApp::kWindowWidth_, WinApp::kWindowHeight_);
	ModelManager::GetInstance()->Initialize();

	Input* input = Input::GetInstance();
	input->Initilize(winApp);



	///
	/// メインループ
	///
	// ウィンドウのｘボタンが押されるまでループ
	while (!winApp->ProcessMessage())
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Engine");
		input->Update();

		///
		/// 更新処理ここから
		///



		///
		/// 更新処理ここまで
		///

		ImGui::End();
		dxCommon->PreDraw();

		///
		/// 描画ここから
		///
		ModelManager::GetInstance()->PreDraw();



		Sprite::PreDraw();

		///
		/// 描画ここまで
		///

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		dxCommon->PostDraw();
	}

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	winApp->Filalze();

	return 0;
}

