#pragma once

#include <Core/DXCommon/DXCommon.h>



class Dissolve
{
public:

    static Dissolve* GetInstance()
    {
        static Dissolve instance;
        return &instance;
    }

    void Initialize();

    void Set();

    void SetThreshold(float _threshold) { thresholdData_->threshold = _threshold; }
    void SetMaskColor(const Vector3& _color) { thresholdData_->maskColor = _color; }
    void SetEnableEdgeColor(bool _enable) { thresholdData_->enableEdgeColor = _enable; }
    void SetEdgeColor(const Vector3& _color) { thresholdData_->edgeColor = _color; }
    void SetEdgeDitectRange(const Vector2& _range) { thresholdData_->edgeDitectRange = _range; }
    void SetTextureHandle(uint32_t _textureHandle) { textureHandle_ = _textureHandle; }

private:

    void CreatePSOForDissolve();


    struct ThresholdBuffer
    {
        float threshold = 0.5f; // マスクを適用するしきい値
        Vector3 maskColor = { 0.0f, 0.0f, 0.0f };

        int enableEdgeColor = 0; // エッジカラーを有効にするかどうか
        Vector3 edgeColor = { 0.0f, 0.0f, 0.0f }; // エッジカラー

        float edgeDitectRange =0.03f; // エッジ検出の範囲
        Vector3 pad;
    };

    Microsoft::WRL::ComPtr<ID3D12Resource> thresholdBuffer_ = nullptr; // 閾値用のバッファ
    ThresholdBuffer* thresholdData_ = nullptr; // CPU側のデータ

    uint32_t textureHandle_ = 0; // テクスチャハンドル mask用



private:
    Dissolve() = default;
    ~Dissolve() = default;
    Dissolve(const Dissolve&) = delete;
    Dissolve& operator=(const Dissolve&) = delete;

};