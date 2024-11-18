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

    void Initialize(CatmulRomSpline* _edit);
    void Update();
    void Draw(const Camera* _camera);

    void CalculateRail();

private:

    std::vector<Vector3> trans_;
    std::vector<Vector3> rot_;

    Model* model_ = nullptr;
    WorldTransform worldTransform_ = {};

    float scale_ = 0.3f;
    float speed_ = 25.0f;
    struct constantBufferData
    {
        Matrix4x4 matWorld;
        Vector4 color;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
    constantBufferData* constMap_;
    uint32_t textureHandle_;
    uint32_t instanceNum_;
    uint32_t srvIndex_;

    CatmulRomSpline* edit_ = nullptr;

    ID3D12PipelineState* pipelineState_;
    ID3D12RootSignature* rootsignature_;

#ifdef _DEBUG
    void ImGui();
#endif // _DEBUG

};