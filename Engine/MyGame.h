#pragma once

#include <Framework.h>

class MyGame : public Framework
{
public:
    MyGame() = default;
    ~MyGame() = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void Finalize() override;

private:

};