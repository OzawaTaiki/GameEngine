#pragma once

#include "Vector4.h"

#include <d3d12.h>
#include <wrl.h>


class ObjectColor
{
public:

    ObjectColor() = default;
    ~ObjectColor() = default;

    void Initialize();

    void SetColor(const Vector4& _color);
    Vector4 GetColor() const { return constMap_->color; }

    void QueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index)const;
    ID3D12Resource* GetResource() { return resource_.Get(); }

private:

    struct ConstantBufferData
    {
        Vector4 color;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> resource_ = nullptr;
    ConstantBufferData* constMap_ = nullptr;
};