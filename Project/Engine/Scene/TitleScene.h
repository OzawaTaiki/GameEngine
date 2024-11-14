#pragma once
#include "BaseScene.h"
#include <memory>
#include <string>

class TitleScene : public BaseScene
{
public:

    static std::unique_ptr<BaseScene> Create();

     ~TitleScene() override;

    void Initialize() override;
    void Update() override;
    void Draw() override;

private:

#ifdef _DEBUG
    std::string nextSceneName_ = {};
    void ImGui();
#endif // _DEBUG
};