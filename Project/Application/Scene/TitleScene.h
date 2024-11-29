#pragma once
#include "eScene/BaseScene.h"
#include <memory>

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
    void ImGui();
#endif // _DEBUG
};