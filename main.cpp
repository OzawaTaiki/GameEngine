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
#include "Sprite.h"

const float kDeltaTime = 1.0f / 60.0f;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{


	WinApp* winApp = WinApp::GetInstance();
	winApp->Initilize();

	DXCommon* dxCommon =  DXCommon::GetInstance();
	dxCommon->Initialize(winApp,WinApp::kWindowWidth_, WinApp::kWindowHeight_);

	TextureManager::GetInstance()->Initialize();
	//TextureManager::GetInstance()->Load("uvChecker.png");

	PSOManager::GetInstance()->Initialize();
	Sprite::StaticInitialize();
	ModelManager::GetInstance()->Initialize();
	//Model* model = Model::CreateFromObj("plane.obj");
	//Model* model = Model::CreateFromObj("bunny.obj");
	Model* model1 = Model::CreateFromObj("plane.obj");
	Model* model2 = Model::CreateFromObj("bunny.gltf");
	Model* model3 = Model::CreateFromObj("bunny.gltf");

	Sprite* sprite = Sprite::Create(0);
	sprite->Initialize();

	Input* input = Input::GetInstance();
	input->Initilize(winApp);

	Camera* camera = new Camera;
	camera->Initialize();

	WorldTransform transform[3];
	for (int i = 0; i < 3; i++)
	{
		transform[i].Initialize();
		transform[i].TransferData(camera->GetViewProjection());
	}

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
		ImGui::DragFloat3("s1", &transform[0].scale_.x, 0.01f);
		ImGui::DragFloat3("r1", &transform[0].rotate_.x, 0.01f);
		ImGui::DragFloat3("t1", &transform[0].transform_.x, 0.01f);
		ImGui::DragFloat3("s2", &transform[1].scale_.x, 0.01f);
		ImGui::DragFloat3("r3", &transform[1].rotate_.x, 0.01f);
		ImGui::DragFloat3("t3", &transform[1].transform_.x, 0.01f);
		ImGui::DragFloat3("s4", &transform[2].scale_.x, 0.01f);
		ImGui::DragFloat3("r4", &transform[2].rotate_.x, 0.01f);
		ImGui::DragFloat3("t4", &transform[2].transform_.x, 0.01f);

		ImGui::DragFloat2("ss", &sprite->scale_.x, 0.01f);
		ImGui::DragFloat("sr", &sprite->rotate_, 0.01f);
		ImGui::DragFloat2("st", &sprite->translate_.x, 0.01f);
		ImGui::End();
		transform[0].rotate_.y += 0.01f;
		transform[0].TransferData(camera->GetViewProjection());
		transform[1].TransferData(camera->GetViewProjection());
		transform[2].TransferData(camera->GetViewProjection());

		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();

		///
		/// 描画ここから
		///
		ModelManager::GetInstance()->PreDraw();

		model1->Draw(transform[0], camera, 0, color);
		model2->Draw(transform[1], camera, 0, color);
		model3->Draw(transform[2], camera, 0, color);

		Sprite::PreDraw();
		sprite->Draw();

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

