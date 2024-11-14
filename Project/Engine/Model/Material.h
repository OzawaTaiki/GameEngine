#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"

#include <cstdint>

#include <d3d12.h>
#include <wrl.h>
#include <string>

class Material
{
public:

    /// uvTransform
    Vector2         transform_                      = {0.0f, 0.0f};         // 移動
    Vector2         scale_                          = { 1.0f, 1.0f };       // スケール
    float           rotation_                       = 0.0f;                 // 回転

    float           shiness_                        = 40.0f;                //

    bool            enableLighting_                 = true;                 // ライティングの有無

    void Initialize(const std::string& _texturepath);

    uint32_t GetTexturehandle() const { return textureHandle_; }
    ID3D12Resource* GetResource() { return resorces_.Get(); }


    void TransferData();
    void MateriallQueueCommand(ID3D12GraphicsCommandList* _commandList,UINT _index) const;
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const;
    void TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index,uint32_t _textureHandle) const;

private:

    std::string     name_                           = {};
    std::string     texturePath_                    = {};
    uint32_t        textureHandle_                  = 0;

    struct DataForGPU
    {
        Matrix4x4       uvTransform;
        float           shininess;
        int32_t         enabledLighthig;
        float           padding[2];
    };

    Microsoft::WRL::ComPtr<ID3D12Resource>          resorces_               = nullptr;
    DataForGPU*                                     constMap_               = nullptr;;

    void LoadTexture();

};