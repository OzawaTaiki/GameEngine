#pragma once

#include <Framework/Framework.h>

class SampleFramework : public Engine::Framework
{
public:
    SampleFramework() = default;
    ~SampleFramework() = default;

    void Initialize(const std::wstring& _winTitle) override;
    void Update() override;
    void Draw() override;

    void Finalize() override;

private:
    // 動画用
    void RenderUI();
    std::map<std::string, std::string> redertextureName_;
    std::string currentTex_;
    std::vector<std::string> activeEffects;

};
