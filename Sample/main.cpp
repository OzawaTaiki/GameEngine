#include "Core/WinApp/WinApp.h"
#include "Core/DirectX/DXCommon.h"
#include "Systems/Input/Input.h"
#include "ResourceManagement/TextureManager/TextureManager.h"
#include "Rendering/Model/ModelManager.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/LineDrawer/LineDrawer.h"
#include "ResourceManagement/SRVManager.h"
#include "Core/DirectX/PSOManager.h"
#include "UI/ImGuiManager/ImGuiManager.h"
#include "Framework/Particle/ParticleManager.h"
#include "Systems/Utility/RandomGenerator.h"
#include "Systems/Time/Time.h"

/*-----シーン-----*/
#include "Framework/eScene/SceneManager.h"
#include "Framework/eScene/SampleScene.h"
#include "Framework/eScene/ParticleTestScene.h"
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

	//ConfigManager::GetInstance()->Initialize();
	//ConfigManager::GetInstance()->LoadData();


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

    JsonHub::GetInstance()->Initialize("Resources/Data/");

	SceneManager::RegisterScene("Sample", SampleScene::Create);
    SceneManager::RegisterScene("ParticleTest", ParticleTestScene::Create);


	SceneManager::GetInstance()->Initialize("Sample");

	Time::Initialize();

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

