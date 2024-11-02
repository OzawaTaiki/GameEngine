#include "SceneManager.h"
#include "GameScene.h"

void SceneManager::Initialize(sceneName _startScene)
{
	previous_ = _startScene;
	current_ = _startScene;
	next_ = current_;

	switch (_startScene)
	{
	case sceneName::Game:
		currentScenePtr_ = new GameScene();
		break;
	default:
		break;
	}
	currentScenePtr_->Initialize();
}

void SceneManager::Update()
{
#ifdef _DEBUG
	//ImGui::Begin("sceneManager");
	//ImGui::End();
#endif // _DEBUG


	if (isReserve_)
	{
		switch (current_)
		{
		case sceneName::Game:
			ReserveScene(sceneName::Result);
			break;
		default:
			break;
		}
	}
}

void SceneManager::Draw()
{
	currentScenePtr_->Draw();
}

void SceneManager::ReserveScene(sceneName _nextScene)
{
	previous_ = current_;
	next_ = _nextScene;
}

void SceneManager::ChangeScene()
{
	current_ = next_;

	delete currentScenePtr_;
	currentScenePtr_ = nullptr;

	switch (current_)
	{
	case sceneName::Game:
		currentScenePtr_ = new GameScene();
		break;
	default:
		break;
	}

	currentScenePtr_->Initialize();

}
