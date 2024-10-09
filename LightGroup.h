#pragma once

#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include <d3d12.h>
#include <memory>


class LightGroup
{
public:
    LightGroup() = default;
    ~LightGroup() = default;

    void Initialize();
    void Update();
    void Draw();

    DirectionalLight* GetDirectoinalLight() { return directoinalLight_.get(); }
    PointLight* GetPointLight() { return pointLight_.get(); }
    SpotLight* GetSpotLight() { return spotLight_.get(); }

    void TransferData() const;

private:

    ID3D12GraphicsCommandList* commandList_ = nullptr;

    std::unique_ptr<DirectionalLight> directoinalLight_ = nullptr;
    std::unique_ptr<PointLight> pointLight_ = nullptr;
    std::unique_ptr<SpotLight> spotLight_ = nullptr;

};