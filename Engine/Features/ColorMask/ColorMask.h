#pragma once

 #include <Math/Matrix/Matrix4x4.h>
#include <Math/Vector/Vector4.h>
#include <Features/UVTransform/UVTransform.h>

#include <cstdint>
#include <memory>
#include <string>

#include <wrl.h>
#include <d3d12.h>

struct ColorMaskData
{
    Matrix4x4 monoTexUVTransform;
    Matrix4x4 colorTexUVTransform;
};

class ColorMask
{
public:
    ColorMask() = default;
    ~ColorMask() = default;

    static ColorMask* GetInstance();
    static void Initialize();

    static void DisPatch(uint32_t _monoTexHandle, uint32_t _colorTexHandle, uint32_t _outputTexHandle, ColorMaskData _data, ID3D12GraphicsCommandList* _commandList, uint32_t _width = 512, uint32_t _height = 512);

    static uint32_t CreateOutputTexture(const std::string& _name, uint32_t _width = 512, uint32_t _height = 512, const Vector4& _clearColor = { 0,0,0,0 }, DXGI_FORMAT _format = DXGI_FORMAT_R8G8B8A8_UNORM);

    static uint32_t GetOutputTextureHandle(const std::string& _name);

    static void Finalize();

private:


    void CreatePipelineState();
    void CreateRootSignature();

    void CreateConstantBuffer();
private:

    static std::unique_ptr<ColorMask> instance_;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;

    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    ColorMaskData* constantBufferMap_ = nullptr;
    //          name        UAV
    std::map<std::string, uint32_t> textureMap_; // テクスチャ名とハンドルのマップ
    //          UAV     RTV
    std::map<uint32_t, uint32_t> outputTextures_; // テクスチャ名とハンドルのマップ
    //          SRV      UAV
    std::map<uint32_t, uint32_t> textureSRVMap_; // テクスチャ名とハンドルのマップ

private:
    ColorMask(const ColorMask&) = delete;
    ColorMask& operator=(const ColorMask&) = delete;


};