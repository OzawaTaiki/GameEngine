#include "EnemyManager.h"
#include "Enemy.h"
#include <fstream>
#include <Windows.h>

EnemyManager* EnemyManager::GetInstance()
{
    static EnemyManager instance;
    return &instance;
}

void EnemyManager::Initialize(const std::string& _loadPath)
{
    LoadPopData(_loadPath);

    popTimer_ = 0.0f;
    popInterval_ = 0.0f;
    enemies_.clear();
}

void EnemyManager::Update()
{
#ifdef _DEBUG
    ImGui();
#endif // _DEBUG

    UpdatePopCommand();

    for (auto& enemy : enemies_)
    {
        enemy.Update();
    }
}

void EnemyManager::Draw(const Camera* _camera)
{
    for (auto& enemy : enemies_)
    {
        enemy.Draw(_camera);
    }
}

void EnemyManager::LoadPopData(const std::string& _loadPath)
{
    // Load file
    std::ifstream ifs(_loadPath);
    // If file not found
    if (!ifs)
    {
        MessageBoxA(nullptr, "File not found", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::stringstream buffer; // 新しいバッファを作成
    buffer << ifs.rdbuf();    // ファイルの内容をバッファに読み込む
    popCommand_.swap(buffer);  // popCommand_と新しいバッファを交換

    // Close file
    ifs.close();
}

void EnemyManager::UpdatePopCommand()
{
    const float kDeltaTime = 1.0f / 60.0f;

    // Timer
    popTimer_ += kDeltaTime;

    if (popInterval_ <= popTimer_)
    {
        popTimer_ = 0.0f;
        std::string line;
        while (std::getline(popCommand_, line))
        {
            std::istringstream line_stream(line);
            std::string word;
            // Load command
            std::getline(line_stream, word, ',');

            // If command is "pop"
            if (word.find("pop") == 0)
            {
                // Load position
                getline(line_stream, word, ',');
                popPos_.x = static_cast<float>(std::atof(word.c_str()));
                getline(line_stream, word, ',');
                popPos_.y = static_cast<float>(std::atof(word.c_str()));
                getline(line_stream, word, ',');
                popPos_.z = static_cast<float>(std::atof(word.c_str()));

                getline(line_stream, word, ',');
                velocity_.x = static_cast<float>(std::atof(word.c_str()));
                getline(line_stream, word, ',');
                velocity_.y = static_cast<float>(std::atof(word.c_str()));
                getline(line_stream, word, ',');
                velocity_.z = static_cast<float>(std::atof(word.c_str()));

                getline(line_stream, word, ',');
                LifeTime_ = static_cast<float>(std::atof(word.c_str()));

                // Create enemy
                PopEnemy(popPos_, velocity_, LifeTime_);

            }
            else if (word.find("wait") == 0)
            {
                // Load interval
                getline(line_stream, word, ',');
                popInterval_ = static_cast<float>(std::atof(word.c_str()));
                break;
            }
        }
    }
}

void EnemyManager::PopEnemy(const Vector3& _pos, const Vector3& _velocity, float _lifeTime)
{
    // Create enemy
    enemies_.emplace_back();
    enemies_.back().Initialize(_pos, _velocity, _lifeTime);

}

#ifdef _DEBUG
#include <imgui.h>
void EnemyManager::ImGui()
{
}
#endif // _DEBUG
