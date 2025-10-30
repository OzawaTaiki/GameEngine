#pragma once

#include <Features/Camera/Camera/Camera.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

#include <d3d12.h>
#include <wrl.h>
#include <array>


class LineDrawer
{
public:

    static LineDrawer* GetInstance();

    void Initialize();
    void SetCameraPtr(const Camera* _cameraPtr) { cameraFor3dptr_ = _cameraPtr; }
    void SetCameraPtr2D(const Camera* _cameraPtr) { cameraFor2dptr_ = _cameraPtr; }
    void SetColor(const Vector4& _color) { color_ = _color; }
    void RegisterPoint(const Vector3& _start, const Vector3& _end, bool _frontDraw = false);
    void RegisterPoint(const Vector3& _start, const Vector3& _end,const Vector4& _color, bool _frontDraw = false);
    void RegisterPoint(const Vector2& _start, const Vector2& _end);
    void RegisterPoint(const Vector2& _start, const Vector2& _end, const Vector4& _color);
    void Draw();

    void DrawOBB(const Matrix4x4& _affineMat, bool _frontDraw = false);
    void DrawOBB(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw = false);
    void DrawOBB(const std::array <Vector3, 8>& _vertices, bool _frontDraw = false);
    void DrawOBB(const std::array <Vector3, 8>& _vertices, const Vector4& _color, bool _frontDraw = false);

    void DrawSphere(const Matrix4x4& _affineMat, bool _frontDraw = false);
    void DrawSphere(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw = false);
    void DrawCircle(const Vector3& _center, float _radius, const float _segmentCount, const Vector3& _normal, bool _frontDraw = false);
    void DrawCircle(const Vector3& _center, float _radius, const float _segmentCount, const Vector3& _normal, const Vector4& _color, bool _frontDraw = false);


    //void DebugDraw(const Vector3& _start, const Vector3& _end, const Vector4& _color);
    void DebugDraw(const Vector2& _start, const Vector2& _end, const Vector4& _color = Vector4{ 0,1,0,1 });
private:
    void Draw3Dlines();
    void Draw2Dlines();
    void Draw3DlinesAlways();
    void DrawDebugLine();

    void TransferData();

    PSOFlags psoFlags_;

    const uint32_t kMaxNum = 4096u * 24u;
    uint32_t indexFor3d_ = 0u;
    uint32_t indexFor3dAlways_ = 0u;
    uint32_t indexFor2d_ = 0u;
    Vector4 color_ = { 0,0,0,1 };
    const Camera* cameraFor3dptr_=nullptr;
    const Camera* cameraFor2dptr_ = nullptr;
    ID3D12RootSignature* rootSignature_ = nullptr;
    ID3D12PipelineState* graphicsPipelineState_ = nullptr;

    // 常時描画のためのPipelineStateObject
    ID3D12PipelineState* graphicsPipelineStateForAlways_ = nullptr;

    struct ConstantBufferData
    {
        Matrix4x4 vp;
    };
    Microsoft::WRL::ComPtr <ID3D12Resource> resourcesForMat3D_ = nullptr;
    ConstantBufferData* matFor3dConstMap_;

    Microsoft::WRL::ComPtr <ID3D12Resource> resourceForMat2D_ = nullptr;
    ConstantBufferData* matFor2dConstMap_;

    struct PointData
    {
        Vector4 position;
        Vector4 color;
    };

    PointData* vConstMapFor3D_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResourceFor3D_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferViewFor3D_ = {};

    PointData* vConstMapFor2D_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResourceFor2D_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferViewFor2D_ = {};

    PointData* vConstMapFor3DAlways_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResourceFor3DAlways_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferViewFor3DAlways_ = {};

#ifdef _DEBUG
    PointData* vConstMapForDebug_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertexResourceForDebug_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW                    vertexBufferViewForDebug_ = {};
    uint32_t indexForDebug_ = 0u;
    void CreateDebugResources();
#endif

    void SetVerties();

    std::array <Vector3, 8> obbVertices_;
    std::array <uint32_t, 24> obbIndices_;
    std::array <Vector3, 32> circleVertices_;

    const float kDivision = 8;
    std::vector <Vector3> sphereVertices_;
    std::vector <uint32_t> sphereIndices_;

private:

    LineDrawer() = default;
    ~LineDrawer() = default;
    LineDrawer(const LineDrawer& ) = delete;
    LineDrawer& operator=(const LineDrawer&) = delete;
};
