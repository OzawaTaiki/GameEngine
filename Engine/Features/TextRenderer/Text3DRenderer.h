#pragma once

#include <Features/TextRenderer/AtlasData.h>
#include <Features/Camera/Camera/Camera.h>

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <d3d12.h>
#include <wrl.h>

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#ifdef DrawText
#undef DrawText
#endif // DrawText


namespace Engine {

class Text3DRenderer
{
public:
    static Text3DRenderer* GetInstance();

    void Initialize(ID3D12Device* _device, ID3D12GraphicsCommandList* _cmdList);
    void Finalize();

    void BeginFrame();
    void EndFrame();

    /// <summary>
    /// 3D空間にテキストを描画
    /// </summary>
    /// <param name="_text">描画するテキスト</param>
    /// <param name="_atlas">使用するフォントアトラス</param>
    /// <param name="_camera">カメラ</param>
    /// <param name="_position">3D位置</param>
    /// <param name="_rotation">回転（ラジアン）</param>
    /// <param name="_scale">スケール</param>
    /// <param name="_color">色</param>
    /// <param name="_pivot">ピボット（0~1）</param>
    void DrawText3D(
        const std::wstring& _text,
        AtlasData* _atlas,
        const Camera* _camera,
        const Vector3& _position,
        const Vector3& _rotation = Vector3::zero,
        const Vector2& _scale = {1.0f, 1.0f},
        const Vector4& _color = Vector4(1, 1, 1, 1),
        const Vector2& _pivot = {0.5f, 0.5f}
    );

    /// <summary>
    /// グラデーション付き3Dテキストを描画
    /// </summary>
    void DrawText3D(
        const std::wstring& _text,
        AtlasData* _atlas,
        const Camera* _camera,
        const Vector3& _position,
        const Vector3& _rotation,
        const Vector2& _scale,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        const Vector2& _pivot = {0.5f, 0.5f}
    );

    /// <summary>
    /// 3Dテキストを即座に描画（逐次描画）
    /// BeginFrame/EndFrameを使わずに即座にGPU転送と描画を実行
    /// </summary>
    void DrawText3DImmediate(
        const std::wstring& _text,
        AtlasData* _atlas,
        const Camera* _camera,
        const Vector3& _position,
        const Vector3& _rotation = Vector3::zero,
        const Vector2& _scale = {1.0f, 1.0f},
        const Vector4& _color = Vector4(1, 1, 1, 1),
        const Vector2& _pivot = {0.5f, 0.5f}
    );

    /// <summary>
    /// グラデーション付き3Dテキストを即座に描画（逐次描画）
    /// </summary>
    void DrawText3DImmediate(
        const std::wstring& _text,
        AtlasData* _atlas,
        const Camera* _camera,
        const Vector3& _position,
        const Vector3& _rotation,
        const Vector2& _scale,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        const Vector2& _pivot = {0.5f, 0.5f}
    );

private:
    struct TextVertex
    {
        Vector4 position = { 0,0,0,1 };
        Vector2 texCoord = { 0,0 };
        Vector4 color = { 1,1,1,1 };
    };

    struct CameraBuffer
    {
        Matrix4x4 view;
        Matrix4x4 projection;
        Vector3 position;
        float padding;
    };

    struct ResourceDataGroup
    {
        // 描画用データ
        std::vector<TextVertex> vertices_;
        std::vector<Matrix4x4> worldMatrices_;

        // GPU リソース
        D3D12_VERTEX_BUFFER_VIEW vbv_;
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D12Resource> matrixBuffer_;

        uint32_t matrixSRVIndex_ = UINT32_MAX;
        uint32_t textureIndex_ = UINT32_MAX;

        AtlasData* atlasData_ = nullptr;
    };

    // 内部描画関数
    void DrawTextInternal(
        const std::wstring& _text,
        const Vector3& _position,
        const Vector3& _rotation,
        const Vector2& _scale,
        const Vector2& _pivot,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        ResourceDataGroup* _res,
        const Camera* _camera
    );

    // リソース管理
    ResourceDataGroup* EnsureAtlasResources(AtlasData* _atlas);
    void CreateVertexBuffer(ResourceDataGroup* _res);
    void CreateMatrixBuffer(ResourceDataGroup* _res);
    void CreateCameraBuffer();
    void CreatePipelineState();
    void CreateRootSignature();

    // GPU転送・描画
    void UploadVertexData(ResourceDataGroup* _res);
    void UploadMatrixData(ResourceDataGroup* _res);
    void UpdateCameraBuffer(const Camera* _camera);
    void RenderText(ResourceDataGroup* _res);

    // 逐次描画用の内部関数
    void CreateImmediateResources();
    void DrawTextImmediate_Internal(
        const std::wstring& _text,
        AtlasData* _atlas,
        const Camera* _camera,
        const Vector3& _position,
        const Vector3& _rotation,
        const Vector2& _scale,
        const Vector4& _topColor,
        const Vector4& _bottomColor,
        const Vector2& _pivot
    );

private:
    ID3D12Device* device_ = nullptr;
    ID3D12GraphicsCommandList* cmdList_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer_;
    CameraBuffer* cameraBufferMap_ = nullptr;

    // バッチ描画用リソース
    size_t maxVertices_ = 9000; // 1500文字 * 6頂点
    size_t maxCharacters_ = 1500;
    std::unordered_map<uint32_t, std::unique_ptr<ResourceDataGroup>> resourceDataGroups_;

    // 逐次描画専用リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> immediateVertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> immediateMatrixBuffer_;
    TextVertex* immediateVertexMap_ = nullptr;  // Persistent mapping
    Matrix4x4* immediateMatrixMap_ = nullptr;   // Persistent mapping
    uint32_t immediateSRVIndex_ = UINT32_MAX;
    D3D12_VERTEX_BUFFER_VIEW immediateVBV_;
    size_t immediateMaxVertices_ = 600;  // 100文字分
    size_t immediateMaxCharacters_ = 100;

    // Singleton
    Text3DRenderer() = default;
    ~Text3DRenderer() = default;
    Text3DRenderer(const Text3DRenderer&) = delete;
    Text3DRenderer& operator=(const Text3DRenderer&) = delete;
    Text3DRenderer(Text3DRenderer&&) = delete;
    Text3DRenderer& operator=(Text3DRenderer&&) = delete;
};

} // namespace Engine
