#include "FFTCS.h"

#include <cmath>
#include <numbers>
#include <chrono>
#include <format>

#include <Core/DXCommon/DXCommon.h>
#include <Core/DXCommon/SRVManager/SRVManager.h>
#include <Core/DXCommon/PSOManager/PSOManager.h>

#include <Debug/Debug.h>

Engine::FFTCS::~FFTCS()
{
    if (mappedUploadBuf_)
    {
        uploadBuffer_->Unmap(0, nullptr);
        mappedUploadBuf_ = nullptr;
    }

    if (fenceEvent_)
    {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
}

void Engine::FFTCS::Initialize(uint32_t fftSize)
{
    fftSize_ = fftSize;

    bits_ = static_cast<uint32_t>(std::log2(fftSize));

    dxCommon_   = DXCommon::GetInstance();
    srvManager_ = SRVManager::GetInstance();

    CreatePipeline();
    CreateBuffers();

    auto* device = dxCommon_->GetDevice();
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                              commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    commandList_->Close();
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);

}

void Engine::FFTCS::Execute(const std::vector<float>& input, std::vector<float>& magnitudeOut)
{
    using clock = std::chrono::high_resolution_clock;
    auto us = [](auto a, auto b)
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(b - a).count();
        };
    auto t0 = clock::now();

    // ハニング窓とビット反転を適用してアップロードバッファに書き込む
    for (uint32_t i = 0; i < fftSize_; ++i)
    {
        uint32_t srcIdx = BitReverse(i, bits_);
        float sample = (srcIdx < input.size()) ? input[srcIdx] : 0.0f;
        float window = HanningWindowValue(srcIdx, fftSize_);
        mappedUploadBuf_[i] = { sample * window, 0.0f };
    }

    auto t1 = clock::now(); // Upload完了

    // コマンドリスト
    commandAllocator_->Reset();
    commandList_->Reset(commandAllocator_.Get(), nullptr);

    // DXCommon::ChangeStateはコマンドリストが固定のため使用しない

    // ヘルパ：バリアの作成
    auto transitionBarrier = [](ID3D12Resource* resource,
                                D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource   = resource;
            barrier.Transition.StateBefore = before;
            barrier.Transition.StateAfter  = after;
            return barrier;
        };
    // ヘルパ：UAVバリアの作成
    auto uavBarrier = [](ID3D12Resource* resource)
        {
            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource          = resource;
            return barrier;
        };

    // uploadBuffer_ → bufferA_コピー
    D3D12_RESOURCE_STATES stateA = buffersInUAVState_ ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON;

    auto barrier = transitionBarrier(bufferA_.Get(), stateA, D3D12_RESOURCE_STATE_COPY_DEST);
    commandList_->ResourceBarrier(1, &barrier);

    commandList_->CopyResource(bufferA_.Get(), uploadBuffer_.Get());

    barrier = transitionBarrier(bufferA_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList_->ResourceBarrier(1, &barrier);

    if (!buffersInUAVState_)
    {// 初回は bufferB_ も UAV 状態に遷移させる
        barrier = transitionBarrier(bufferB_.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        commandList_->ResourceBarrier(1, &barrier);
        buffersInUAVState_ = true;
    }

    srvManager_->PreDraw(commandList_.Get());
    commandList_->SetPipelineState(pipeline_.Get());
    commandList_->SetComputeRootSignature(rootSignature_.Get());

    const uint32_t dispatchCount = (fftSize_ / 2 + 255) / 256;

    for (uint32_t stage = 0; stage < bits_; ++stage)
    {
        // 偶数ステージ: A→B、奇数ステージ: B→A
        uint32_t inputUav   = (stage % 2 == 0) ? uavIndexA_ : uavIndexB_;
        uint32_t outputUav  = (stage % 2 == 0) ? uavIndexB_ : uavIndexA_;
        ID3D12Resource* outputRes = (stage % 2 == 0) ? bufferB_.Get() : bufferA_.Get();

        // RootConstants: { fftSize, currentStage, isInverse=0, padding=0 }
        uint32_t consts[4] = { fftSize_, stage, 0u, 0u };
        commandList_->SetComputeRoot32BitConstants(0, 4, consts, 0);

        // bitReversalIndex SRV
        commandList_->SetComputeRootDescriptorTable(1, srvManager_->GetGPUSRVDescriptorHandle(srvBitRevIndex_));

        // input UAV (u0)
        commandList_->SetComputeRootDescriptorTable(2, srvManager_->GetGPUSRVDescriptorHandle(inputUav));

        // output UAV (u1)
        commandList_->SetComputeRootDescriptorTable(3, srvManager_->GetGPUSRVDescriptorHandle(outputUav));

        commandList_->Dispatch(dispatchCount, 1, 1);

        // 動作順を保証するための UAV バリア
        auto uavB = uavBarrier(outputRes);
        commandList_->ResourceBarrier(1, &uavB);
    }
    // 最終結果を Readback にコピー
    // bits_ が偶数 → bufferA_ が最終出力
    // bits_ が奇数 → bufferB_ が最終出力
    ID3D12Resource* finalBuffer = (bits_ % 2 == 0) ? bufferA_.Get() : bufferB_.Get();

    barrier = transitionBarrier(finalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
    commandList_->ResourceBarrier(1, &barrier);

    commandList_->CopyResource(readbackBuffer_.Get(), finalBuffer);

    barrier = transitionBarrier(finalBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    commandList_->ResourceBarrier(1, &barrier);

    commandList_->Close();

    auto t2 = clock::now(); // CL記録完了

    // GPU 実行
    ID3D12CommandList* lists[] = { commandList_.Get() };
    dxCommon_->GetCommandQueue()->ExecuteCommandLists(1, lists);
    // GPU の完了を待つ
    ++fenceValue_;
    dxCommon_->GetCommandQueue()->Signal(fence_.Get(), fenceValue_);
    fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
    WaitForSingleObject(fenceEvent_, INFINITE);

    auto t3 = clock::now(); // GPU待機完了

    // まぐにちゅーど計算のために CPU に結果をコピー
    GPUComplex* resultPtr = nullptr;
    D3D12_RANGE readRange = { 0, fftSize_ * sizeof(GPUComplex) };
    readbackBuffer_->Map(0, &readRange, reinterpret_cast<void**>(&resultPtr));

    const float windowGain = 2.0f;
    const float fftNorm    = 1.0f / static_cast<float>(fftSize_);

    magnitudeOut.resize(fftSize_ / 2);
    for (uint32_t i = 0; i < fftSize_ / 2; ++i)
    {
        float re = resultPtr[i].real;
        float im = resultPtr[i].imag;
        magnitudeOut[i] = sqrtf(re * re + im * im) * windowGain * fftNorm;
    }

    D3D12_RANGE writeRange = { 0, 0 };
    readbackBuffer_->Unmap(0, &writeRange);

    auto t4 = clock::now(); // 終了

    uploadUs_   = us(t0, t1);
    clUs_       = us(t1, t2);
    gpuUs_      = us(t2, t3);
    readbackUs_ = us(t3, t4);
    totalUs_    = us(t0, t4);
}

void Engine::FFTCS::CreatePipeline()
{
    auto* device = dxCommon_->GetDevice();
    HRESULT hr;

    // ----- RootSignature -----
    // [0] RootConstants 4個    → b0  (fftSize, currentStage, isInverse, padding)
    // [1] DescriptorTable SRV  → t0  (bitReversalIndex)
    // [2] DescriptorTable UAV  → u0  (input  / ping-pong)
    // [3] DescriptorTable UAV  → u1  (output / ping-pong)

    D3D12_DESCRIPTOR_RANGE ranges[3] = {};

    // t0
    ranges[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    ranges[0].NumDescriptors                    = 1;
    ranges[0].BaseShaderRegister                = 0;
    ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // u0
    ranges[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    ranges[1].NumDescriptors                    = 1;
    ranges[1].BaseShaderRegister                = 0;
    ranges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // u1
    ranges[2].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    ranges[2].NumDescriptors                    = 1;
    ranges[2].BaseShaderRegister                = 1;
    ranges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER params[4] = {};

    // [0] RootConstants
    params[0].ParameterType            = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    params[0].ShaderVisibility         = D3D12_SHADER_VISIBILITY_ALL;
    params[0].Constants.ShaderRegister = 0;
    params[0].Constants.RegisterSpace  = 0;
    params[0].Constants.Num32BitValues = 4;

    // [1] SRV t0
    params[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    params[1].DescriptorTable.pDescriptorRanges   = &ranges[0];
    params[1].DescriptorTable.NumDescriptorRanges = 1;

    // [2] UAV u0
    params[2].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[2].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    params[2].DescriptorTable.pDescriptorRanges   = &ranges[1];
    params[2].DescriptorTable.NumDescriptorRanges = 1;

    // [3] UAV u1
    params[3].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[3].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_ALL;
    params[3].DescriptorTable.pDescriptorRanges   = &ranges[2];
    params[3].DescriptorTable.NumDescriptorRanges = 1;

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    rsDesc.NumParameters = _countof(params);
    rsDesc.pParameters   = params;
    rsDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_NONE; // Compute に INPUT_ASSEMBLER 不要

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob, errBlob;
    hr = D3D12SerializeRootSignature(
        &rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &sigBlob, &errBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errBlob->GetBufferPointer()));
        assert(false);
    }
    hr = device->CreateRootSignature(
        0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));

    // ----- ComputePipelineState -----
    // エントリポイントは ButterflyCS (main ではない)
    auto blob = PSOManager::GetInstance()->ComplieShader(
        L"FFT.CS.hlsl", L"cs_6_0", L"ButterflyCS");
    assert(blob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.CS = { blob->GetBufferPointer(), blob->GetBufferSize() };

    hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipeline_));
    assert(SUCCEEDED(hr));

}

void Engine::FFTCS::CreateBuffers()
{
    const uint32_t complexBytes = fftSize_ * sizeof(GPUComplex);
    const uint32_t uintBytes    = fftSize_ * sizeof(uint32_t);

    // Ping-pong バッファ
    bufferA_ = dxCommon_->CreateUAVBufferResource(complexBytes);
    bufferB_ = dxCommon_->CreateUAVBufferResource(complexBytes);

    // アップロードバッファ
    uploadBuffer_ = dxCommon_->CreateBufferResource(complexBytes);
    uploadBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedUploadBuf_));

    // ビット反転テーブル
    bitRevBuffer_ = dxCommon_->CreateBufferResource(uintBytes);
    uint32_t* bitRevPtr = nullptr;
    bitRevBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&bitRevPtr));
    for (uint32_t i = 0; i < fftSize_; ++i)
        bitRevPtr[i] = i;

    bitRevBuffer_->Unmap(0, nullptr);

    // Readback バッファ
    readbackBuffer_ = dxCommon_->CreateReadbackResources(complexBytes);

    // --- SRVManager に登録 ---
    uavIndexA_ = srvManager_->Allocate();
    srvManager_->CreateUAVForBuffer(uavIndexA_, bufferA_.Get(), fftSize_, sizeof(GPUComplex));

    uavIndexB_ = srvManager_->Allocate();
    srvManager_->CreateUAVForBuffer(uavIndexB_, bufferB_.Get(), fftSize_, sizeof(GPUComplex));

    srvBitRevIndex_ = srvManager_->Allocate();
    srvManager_->CreateSRVForStructureBuffer(srvBitRevIndex_, bitRevBuffer_.Get(),
                                             fftSize_, sizeof(uint32_t));
}

uint32_t Engine::FFTCS::BitReverse(uint32_t n, uint32_t bits)
{
    uint32_t reversed = 0;
    for (uint32_t i = 0; i < bits; ++i)
    {
        reversed <<= 1;
        reversed |= (n & 1);
        n >>= 1;
    }
    return reversed;
}

float Engine::FFTCS::HanningWindowValue(uint32_t n, uint32_t N)
{
    if (N == 0)
        return 0.0f;

    return 0.5f * (1.0f - std::cos(2.0f * std::numbers::pi_v<float> *static_cast<float>(n) / static_cast<float>(N - 1)));
}
