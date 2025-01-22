#pragma once

#include <Framework/Framework.h>

class Sample : public Framework
{
public:
    Sample() = default;
    ~Sample() = default;

    void Initialize() override;
    void Update() override;
    void Draw() override;

    void Finalize() override;

private:

};
