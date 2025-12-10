#pragma once

#include <Features/Camera/Camera/Camera.h>
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Framework/LayerSystem/LayerSystem.h>

#include <d3d12.h>
#include <wrl.h>
#include <map>
#include <vector>
#include <array>

/// <summary>
/// レイヤーシステム対応のライン描画クラス
/// 登録時に自動的に現在のレイヤーを取得し、レイヤーごとに描画を分離
/// </summary>
class LineDrawer
{
public:
    static LineDrawer* GetInstance();

    void Initialize();

    /// <summary>
    /// カメラ設定
    /// </summary>
    void SetCameraPtr(const Camera* _cameraPtr) { cameraFor3D_ = _cameraPtr; }
    void SetCameraPtr2D(const Camera* _cameraPtr) { cameraFor2D_ = _cameraPtr; }

    /// <summary>
    /// デフォルト色設定
    /// </summary>
    void SetColor(const Vector4& _color) { color_ = _color; }

    /// <summary>
    /// 3D空間の線を登録（現在のレイヤーに自動追加）
    /// </summary>
    void RegisterPoint(const Vector3& _start, const Vector3& _end, bool _frontDraw = false);
    void RegisterPoint(const Vector3& _start, const Vector3& _end, const Vector4& _color, bool _frontDraw = false);

    /// <summary>
    /// 2D空間の線を登録（現在のレイヤーに自動追加）
    /// </summary>
    void RegisterPoint(const Vector2& _start, const Vector2& _end);
    void RegisterPoint(const Vector2& _start, const Vector2& _end, const Vector4& _color);

    /// <summary>
    /// OBBを描画
    /// </summary>
    void DrawOBB(const Matrix4x4& _affineMat, bool _frontDraw = false);
    void DrawOBB(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw = false);
    void DrawOBB(const std::array<Vector3, 8>& _vertices, bool _frontDraw = false);
    void DrawOBB(const std::array<Vector3, 8>& _vertices, const Vector4& _color, bool _frontDraw = false);

    /// <summary>
    /// 球を描画
    /// </summary>
    void DrawSphere(const Matrix4x4& _affineMat, bool _frontDraw = false);
    void DrawSphere(const Matrix4x4& _affineMat, const Vector4& _color, bool _frontDraw = false);

    /// <summary>
    /// 円を描画
    /// </summary>
    void DrawCircle(const Vector3& _center, float _radius, float _segmentCount, const Vector3& _normal, bool _frontDraw = false);
    void DrawCircle(const Vector3& _center, float _radius, float _segmentCount, const Vector3& _normal, const Vector4& _color, bool _frontDraw = false);

    /// <summary>
    /// 全レイヤーの線を描画
    /// </summary>
    void Draw();

    /// <summary>
    /// デバッグ用
    /// 最前面に描画される
    /// </summary>
    void DebugDraw(const Vector2& start, const Vector2& end, const Vector4& color = { 1,1,1,1 });

    void DebugDrawCircle(const Vector2& center, float radius, const Vector4& color = { 1,1,1,1 });

private:
    LineDrawer() = default;
    ~LineDrawer() = default;
    LineDrawer(const LineDrawer&) = delete;
    LineDrawer& operator=(const LineDrawer&) = delete;

    struct PointData
    {
        Vector4 position;
        Vector4 color;
    };

    /// <summary>
    /// レイヤーごとの描画データ
    /// </summary>
    struct DrawData
    {
        std::vector<PointData> lines3D_;
        std::vector<PointData> lines3DAlways_;
        std::vector<PointData> lines2D_;

        void Clear()
        {
            lines3D_.clear();
            lines3DAlways_.clear();
            lines2D_.clear();
        }

        bool HasData() const
        {
            return !lines3D_.empty() || !lines3DAlways_.empty() || !lines2D_.empty();
        }
    };

    /// <summary>
    /// 描画処理（内部）
    /// </summary>
    void Draw3DLines();
    void Draw3DLinesAlways();
    void Draw2DLines();

    /// <summary>
    /// カメラ行列転送
    /// </summary>
    void TransferCameraMatrix3D();
    void TransferCameraMatrix2D();

    /// <summary>
    /// 形状頂点初期化
    /// </summary>
    void InitializeShapeVertices();

private:
    // レイヤーごとのデータ（自動的にソートされる）
    std::map<LayerID, DrawData> layerData_;

    // カメラ
    const Camera* cameraFor3D_ = nullptr;
    const Camera* cameraFor2D_ = nullptr;

    Camera defaultCamera2D_;


    // デフォルト色
    Vector4 color_ = { 0.0f, 0.0f, 0.0f, 1.0f };

    // PSO
    PSOFlags psoFlags_;
    ID3D12RootSignature* rootSignature_ = nullptr;
    ID3D12PipelineState* graphicsPipelineState_ = nullptr;
    ID3D12PipelineState* graphicsPipelineStateForAlways_ = nullptr;

    // 共有GPUリソース
    static constexpr uint32_t kMaxNum = 4096u * 24u;

    // 定数バッファ（カメラ行列）
    struct ConstantBufferData
    {
        Matrix4x4 vp;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resourcesForMat3D_;
    ConstantBufferData* matFor3DConstMap_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> resourceForMat2D_;
    ConstantBufferData* matFor2DConstMap_ = nullptr;

    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceFor3D_;
    PointData* vConstMapFor3D_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewFor3D_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceFor3DAlways_;
    PointData* vConstMapFor3DAlways_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewFor3DAlways_{};

    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceFor2D_;
    PointData* vConstMapFor2D_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferViewFor2D_{};

    // 形状描画用の頂点データ
    std::array<Vector3, 8> obbVertices_;
    std::array<uint32_t, 24> obbIndices_;
    std::vector<Vector3> sphereVertices_;
    std::vector<uint32_t> sphereIndices_;
    const float kDivision = 8;
};
