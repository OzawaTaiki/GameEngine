#pragma once

#include "model.h"
#include "WorldTransform.h"
#include "Camera.h"

#include <vector>

class CatmulRomSpline;
class Rail
{
public:

    Rail() = default;
    ~Rail() = default;

    void Initialize();
    void Update();
    void Draw(const Camera* _camera);

    void CalculateRail(CatmulRomSpline* _edit);

private:

    std::vector<Vector3> trans_;
    std::vector<Vector3> rot_;

    Model* model_ = nullptr;
    WorldTransform worldTransform_ = {};

    float scale_ = 1.0f;

    struct constantBufferData
    {
        Matrix4x4 matWorld;
        Vector4 color;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    constantBufferData* constMap_;
    uint32_t textureHandle_;
    Model* model_;
    uint32_t instanceNum_;
    uint32_t srvIndex_;

    ID3D12PipelineState* pipelineState_;
    ID3D12RootSignature* rootsignature_;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};