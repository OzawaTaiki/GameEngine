#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <memory>


namespace Engine {

enum class CommandListType {
    Graphics,
    Compute,
    Copy
};

class CommandList {
public:
    CommandList(ID3D12Device* device, CommandListType type);
    ~CommandList();

    // コマンドリストの準備
    void Reset(ID3D12PipelineState* initialState = nullptr);

    // コマンドの実行と同期
    void Execute(bool waitForCompletion = false);

    // リソース状態の遷移
    void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

    // アクセサ
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return commandQueue_.Get(); }

    // GPUの処理完了を待機
    void WaitForGpu();

    // 最終的なフェンス値を取得
    UINT64 GetFenceValue() const { return fenceValue_; }

private:
    // DirectX 12リソース
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    // 同期オブジェクト
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;

    // リソースの状態追跡
    CommandListType type_;
    bool isRecording_ = false;
};

} // namespace Engine
