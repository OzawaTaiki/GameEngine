#pragma once

#include "Vector3.h"
#include "Camera.h"
#include <string>
#include <sstream>

class Enemy;
class EnemyManager
{
public:

    static EnemyManager* GetInstance();

    void Initialize(const std::string& _loadPath);
    void Update();
    void Draw(const Camera* _camera);

private:

    void LoadPopData(const std::string& _loadPath);
    void UpdatePopCommand();

    void PopEnemy(const Vector3& _pos, const Vector3& _velocity, float _lifeTime);

    std::list<Enemy> enemies_;


    /// pop data
    std::stringstream popCommand_;
    float popTimer_ = 0.0f;
    float popInterval_ = 0.0f;
    Vector3 popPos_ = {};
    Vector3 velocity_ = {};
    float LifeTime_ = 0.0f;

    EnemyManager() = default;
    ~EnemyManager() = default;
    EnemyManager(const EnemyManager&) = delete;
    EnemyManager& operator=(const EnemyManager&) = delete;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};