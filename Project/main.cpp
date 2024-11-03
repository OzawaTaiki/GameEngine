#include "WinApp.h"
#include "DXCommon.h"
#include "Input.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "GameScene.h"
#include "LineDrawer.h"
#include "SRVManager.h"
#include "ImGuiManager.h"
#include "ParticleManager.h"
#include "RandomGenerator.h"
#include <random>

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initilize();

	DXCommon* dxCommon =  DXCommon::GetInstance();
	dxCommon->Initialize(winApp,WinApp::kWindowWidth_, WinApp::kWindowHeight_);

	std::unique_ptr<SRVManager> srvManager = std::make_unique<SRVManager>();
	srvManager->Initialize();
	PSOManager::GetInstance()->Initialize();

	std::unique_ptr<ImGuiManager> imguiManager = std::make_unique <ImGuiManager>();
	imguiManager->Initialize(srvManager.get());

	ParticleManager* particle = ParticleManager::GetInstance();
	particle->Initialize(srvManager.get());


	TextureManager::GetInstance()->Initialize(srvManager.get());
	TextureManager::GetInstance()->Load("cube.jpg");
	TextureManager::GetInstance()->Load("uvChecker.png");

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
		imguiManager->Begin();

		///
		/// 更新処理ここから
		///

		gameScene->Update();

		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();
		srvManager->PreDraw();

		///
		/// 描画ここから
		///

		gameScene->Draw();


		///
		/// 描画ここまで
		///
		imguiManager->End();
		imguiManager->Draw();

		dxCommon->PostDraw();
	}
	imguiManager->Finalize();

	winApp->Filalze();

	delete gameScene;

	return 0;
}

