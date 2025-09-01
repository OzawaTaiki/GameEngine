#pragma once
#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <d3d12.h>
#include <wrl.h>

struct WaveformBounds
{
    WaveformBounds() = default;
    WaveformBounds(const Vector2& _leftTop, const Vector2& _size) : leftTop(_leftTop), size(_size) {}
    WaveformBounds(float _left, float _top, float _width, float _height) : leftTop(_left, _top), size(_width, _height) {}

    Vector2 leftTop, size;

    float Right() const { return leftTop.x + size.x; }
    float Bottom() const { return leftTop.y + size.y; }
    Vector2 RightBottom() const { return Vector2(Right(), Bottom()); }
    float CenterX() const { return leftTop.x + size.x * 0.5f; }
    float CenterY() const { return leftTop.y + size.y * 0.5f; }
    Vector2 Center() const { return Vector2(CenterX(), CenterY()); }


};

class SoundInstance;
class LineDrawer;

class WaveformDisplay
{
public:

    WaveformDisplay() = default;
    ~WaveformDisplay() = default;

    void Initialize(const SoundInstance* _soundInstance, const WaveformBounds& _bounds, const Matrix4x4& _matVP);
    void Draw();

    void SetDisplayTimeWindow(float _displayTimeWindow);
    float GetDisplayTimeWindow() const { return displayTimeWindow_; }


    void SetStartTime(float _startTime);
    float GetStartTime() const { return startTime_; }

    void SetSampleRate(float _sampleRate);

    void SetSoundInstance(const SoundInstance* _soundInstance);

    void SetBounds(const WaveformBounds& _bounds) { bounds_ = _bounds; }
    void SetBounds(const Vector2& _leftTop, const Vector2& _size) { bounds_ = WaveformBounds(_leftTop, _size); }
    void SetBounds(float _left, float _top, float _width, float _height) { bounds_ = WaveformBounds(_left, _top, _width, _height); }
    const WaveformBounds& GetBounds() const { return bounds_; }

private:

    void CalculateDisplayRange();

    std::vector<float> LanczosResample(const std::vector<float>& _input, int _outputSize = 1280) const;
    std::vector<float> LanczosResample(const std::vector<float>& _input, size_t _startSample, size_t _enbSample, int _outputSize = 1280) const;

    float LanczosKernel(float x, int kernelSize) const;

    void DrawCenterLine();

    void DrawWaveform();

    void CreatePipeline();
    void CreateConstantBuffer(const Matrix4x4& _matVP);
    void CreateVertexBuffer();


private:

    struct ConstantBufferData
    {
        Vector4 color; // 波形の色

        Vector2 leftTop; // 波形のオフセット
        float startTime; // 波形の開始時間
        float displayDuration;

        float displayHeight; // 描画範囲 の高さ
        float displayWidth; // 描画範囲 の幅

        Vector2 screenSize;

        Matrix4x4 matViewProj = Matrix4x4::Identity(); // ビュー射影行列
    };

private:

    const SoundInstance* soundInstance_ = nullptr;
    LineDrawer* lineDrawer_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_ = nullptr;

    ConstantBufferData* mapData_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;

    // VBV
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    Vector2* mappedVertexBuffer_ = nullptr;
    static const int kMaxVertices_ = 44100 * 150; // sampleRate * 3分
    UINT instanceCount_ = 0;

    WaveformBounds bounds_;
    std::vector<float> waveformCache_; // 波形データのキャッシュ
    bool isValidCache_ = false; // キャッシュが有効かどうか

    float startTime_ = 0.0f; // 開始時間
    float endTime_ = 0.0f;   // 終了時間

    float displayTimeWindow_ = 5.0f; // 表示する時間（秒）

    float sampleRate_ = 44100.0f; // サンプルレート

    static float waveformTimeWindow_;

};
