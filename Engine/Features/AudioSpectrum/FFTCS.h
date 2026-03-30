#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <d3d12.h>
#include <wrl.h>


namespace Engine
{

class DXCommon;
class SRVManager;

class FFTCS
{
public:

    FFTCS() = default;
    ~FFTCS();

    /// <summary>
    /// FFTCSの初期化処理。
    /// </summary>
    /// <param name="fftSize">2のべき乗</param>
    void Initialize(uint32_t fftSize);


    /// <summary>
    /// FFTCSの実行処理。
    /// </summary>
    /// <param name="input">FFTSize個のfloat</param>
    /// <param name="magnitudeOut">fftSize/2個の出力</param>
    void Execute(const std::vector<float>& input, std::vector<float>& magnitudeOut);

    uint64_t GetUploadUs()   const { return uploadUs_;   }
    uint64_t GetCLUs()       const { return clUs_;       }
    uint64_t GetGPUUs()      const { return gpuUs_;      }
    uint64_t GetReadbackUs() const { return readbackUs_; }
    uint64_t GetTotalUs()    const { return totalUs_;    }

private:

    struct GPUComplex
    {
        float real;
        float imag;
    };

private:

    void CreatePipeline();
    void CreateBuffers();

    static uint32_t BitReverse(uint32_t n, uint32_t bits);

    // n: 現在のサンプルインデックス
    // N: 全サンプル数
    static float HanningWindowValue(uint32_t n, uint32_t N);
private:

    uint32_t fftSize_   = 0;
    uint32_t bits_      = 0;

    Microsoft::WRL::ComPtr<ID3D12Resource> bufferA_;
    Microsoft::WRL::ComPtr<ID3D12Resource> bufferB_;
    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> bitRevBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> readbackBuffer_;

    GPUComplex* mappedUploadBuf_ = nullptr;

    uint32_t uavIndexA_      = (std::numeric_limits<uint32_t>::max)();
    uint32_t uavIndexB_      = (std::numeric_limits<uint32_t>::max)();
    uint32_t srvBitRevIndex_ = (std::numeric_limits<uint32_t>::max)();

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;

    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    bool buffersInUAVState_ = false;

    DXCommon* dxCommon_ = nullptr;
    SRVManager* srvManager_ = nullptr;

    // --- タイミング計測結果 ---
    uint64_t uploadUs_   = 0;
    uint64_t clUs_       = 0;
    uint64_t gpuUs_      = 0;
    uint64_t readbackUs_ = 0;
    uint64_t totalUs_    = 0;

};


}// namespace Engine