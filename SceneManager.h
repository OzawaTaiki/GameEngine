#pragma once
#include "Scene.h"
#include <cstdint>

enum class sceneName
{
	Title,
	FromTitle,
	Game,
	Result
};

class SceneManager
{
public:
	static SceneManager*	GetInstance() { static SceneManager instance; return &instance; }

	void					Initialize(sceneName _startScene);
	void					Update();
	void					Draw();


private:
	SceneManager() = default;

	void					ReserveScene(sceneName _nextScene);
	void					ChangeScene();

	sceneName						current_				= static_cast<sceneName>(1);
	sceneName						next_					= static_cast<sceneName>(1);
	sceneName						previous_				= static_cast<sceneName>(1);


	Scene*							currentScenePtr_			=nullptr;


	bool							isSceneChange_				= false;
	bool							isReserve_					= false;

	SceneManager(const SceneManager& _sceneManager) = delete;
	SceneManager& operator=(const SceneManager& _sceneManager) = delete;
};