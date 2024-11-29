#include "Core/WinApp/WinApp.h"
#include "Core/DirectX/DXCommon.h"
#include "Input/Input.h"
#include "TextureManager/TextureManager.h"
#include "Model/ModelManager.h"
#include "Sprite/Sprite.h"
#include "LineDrawer/LineDrawer.h"
#include "Render/SRVManager.h"
#include "Render/PSOManager.h"
#include "ImGuiManager/ImGuiManager.h"
#include "Particle/ParticleManager.h"
#include "Utility/RandomGenerator.h"
#include "Utility/ConfigManager.h"

/*-----シーン-----*/
#include "eScene/SceneManager.h"
#include "Scene/GameScene.h"
#include "Scene/TitleScene.h"
/*---------------*/


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	WinApp* winApp = WinApp::GetInstance();
	winApp->Initilize();

	DXCommon* dxCommon = DXCommon::GetInstance();
	dxCommon->Initialize(winApp, WinApp::kWindowWidth_, WinApp::kWindowHeight_);

	SRVManager* srvManager = SRVManager::GetInstance();
	srvManager->Initialize();
	PSOManager::GetInstance()->Initialize();


	std::unique_ptr<ImGuiManager> imguiManager = std::make_unique <ImGuiManager>();
	imguiManager->Initialize();

	ParticleManager* particle = ParticleManager::GetInstance();
	particle->Initialize();

	ConfigManager::GetInstance()->Initialize();
	ConfigManager::GetInstance()->LoadData();

	TextureManager::GetInstance()->Initialize();
	TextureManager::GetInstance()->Load("white.png");
	TextureManager::GetInstance()->Load("cube.jpg");
	TextureManager::GetInstance()->Load("uvChecker.png");

	Sprite::StaticInitialize(WinApp::kWindowWidth_, WinApp::kWindowHeight_);
	ModelManager::GetInstance()->Initialize();

	LineDrawer* lineDrawer = LineDrawer::GetInstance();
	lineDrawer->Initialize();

	Input* input = Input::GetInstance();
	input->Initilize(winApp);


	SceneManager::RegisterScene("title", TitleScene::Create);
	SceneManager::RegisterScene("game", GameScene::Create);
	SceneManager::GetInstance()->Initialize("title");

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

		SceneManager::GetInstance()->Update();
		//gameScene->Update();

		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();
		srvManager->PreDraw();

		///
		/// 描画ここから
		///

		SceneManager::GetInstance()->Draw();
		//gameScene->Draw();


		///
		/// 描画ここまで
		///
		imguiManager->End();
		imguiManager->Draw();

		dxCommon->PostDraw();

		SceneManager::ChangeScene();
	}
	imguiManager->Finalize();

	winApp->Filalze();


	return 0;
}

