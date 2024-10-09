#include "WinApp.h"
#include "DXCommon.h"
#include "myLib/MyLib.h"
#include "Debug.h"
#include "Input.h"
#include "TextureManager.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include <string>
#include <format>
#include <vector>
#include <fstream>
#include <sstream>
#include <random>
#include <numbers>
#include "PSOManager.h"
#include "ModelManager.h"

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



		
		///
		/// 更新処理ここまで
		///

		dxCommon->PreDraw();

		///
		/// 描画ここから 
		/// 



		

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

