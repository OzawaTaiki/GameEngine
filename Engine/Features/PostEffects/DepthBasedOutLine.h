#pragma once

#include <Features/Camera/Camera/Camera.h>

class DepthBasedOutLine
{
public:
    static DepthBasedOutLine* GetInstance()
    {
        static DepthBasedOutLine instance;
        return &instance;
    }


    void Initialize();

    void Set(const std::string& _depthTextureName);

    void SetCamera(Camera* _camera) { camera_ = _camera; }
    void SetDepthTexture(ID3D12Resource* _depthTexture) { depthTexture_ = _depthTexture; }

private:
    void SetPSO();
    void SetRootSignature();


    void CreatePSOForDepthBasedOutLine();


    Camera* camera_ = nullptr;
    ID3D12Resource* depthTexture_ = nullptr;

    std::string name_ = "DepthBasedOutline";

    Microsoft::WRL::ComPtr<ID3D12Resource> inverseMatrixBuffer_ = nullptr;
    Matrix4x4* inverseMatrixData_ = nullptr;

private:
    DepthBasedOutLine() = default;
    ~DepthBasedOutLine() = default;
    DepthBasedOutLine(const DepthBasedOutLine&) = delete;
    DepthBasedOutLine& operator=(const DepthBasedOutLine&) = delete;
};