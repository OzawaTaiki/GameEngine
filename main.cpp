#include "WinApp.h"
#include "DXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "GameScene.h"
#include "LineDrawer.h"

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

	LineDrawer* lineDrawer = LineDrawer::GetInstance();
	lineDrawer->Initialize();

	Input* input = Input::GetInstance();
	input->Initilize(winApp);

	GameScene* gameScene = new GameScene;
	gameScene->Initialize();

	///
	/// メインループ
	///
	// ウィンドウのｘボタンが押されるまでループ
	while (!winApp->ProcessMessage())
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		///
		/// 更新処理ここから
		///

		gameScene->Update();

		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();

		///
		/// 描画ここから
		///

		gameScene->Draw();

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

	delete gameScene;

	return 0;
}

