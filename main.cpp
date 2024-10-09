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

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include <random>

const float kDeltaTime = 1.0f / 60.0f;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	WinApp* winApp = WinApp::GetInstance();
	winApp->Initilize();

	DXCommon* dxCommon =  DXCommon::GetInstance();
	dxCommon->Initialize(winApp,WinApp::kWindowWidth_, WinApp::kWindowHeight_);

	TextureManager::GetInstance()->Initialize();
	TextureManager::GetInstance()->Load("uvChecker.png");

	PSOManager::GetInstance()->Initialize();

	ModelManager::GetInstance()->Initialize();
	Model* model = Model::CreateFromObj("plane.obj");

	Input* input = Input::GetInstance();
	input->Initilize(winApp);

	Camera* camera = new Camera;
	camera->Initialize();

	WorldTransform transform;
	transform.Initialize();
	transform.TransferData(camera->GetViewProjection());

	ObjectColor* color = new ObjectColor;
	color->Initialize();

	///
	/// メインループ
	/// 
	// ウィンドウのｘボタンが押されるまでループ
	while (1)
	{
		// Windowにメッセージが来ていたら最優先で処理させる
		if (winApp->ProcessMessage())
		{
			break;
		}

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		///
		/// 更新処理ここから
		/// 

		input->Update();

		ImGui::Begin("debug");
		ImGui::DragFloat3("t", &transform.transform_.x, 0.01f);
		ImGui::End();
		transform.TransferData(camera->GetViewProjection());
		
		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();

		///
		/// 描画ここから 
		/// 
		ModelManager::GetInstance()->PreDraw();

		model->Draw(transform, camera, 0, color);

		

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

	delete camera;
	delete color;


	return 0;
}

