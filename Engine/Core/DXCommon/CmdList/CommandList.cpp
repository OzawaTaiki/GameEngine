#include "CommandList.h"
#include <cassert>

CommandList::CommandList(ID3D12Device* device, CommandListType type) : type_(type) {
    HRESULT hr;

    // コマンドリストタイプを変換
    D3D12_COMMAND_LIST_TYPE d3dType = D3D12_COMMAND_LIST_TYPE_DIRECT;
    switch (type)
    {
    case CommandListType::Graphics:
        d3dType = D3D12_COMMAND_LIST_TYPE_DIRECT;
        break;
    case CommandListType::Compute:
        d3dType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        break;
    case CommandListType::Copy:
        d3dType = D3D12_COMMAND_LIST_TYPE_COPY;
        break;
    }

    // コマンドキューの作成
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = d3dType;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    // コマンドアロケーターの作成
    hr = device->CreateCommandAllocator(d3dType, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    // コマンドリストの作成
    hr = device->CreateCommandList(0, d3dType, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));

    // 初期状態ではクローズしておく
    hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    // フェンスの作成
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    // フェンスイベントの作成
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent_ != nullptr);
}

CommandList::~CommandList() {
    if (fenceEvent_) {
        CloseHandle(fenceEvent_);
        fenceEvent_ = nullptr;
    }
}

void CommandList::Reset(ID3D12PipelineState* initialState) {
    // コマンドアロケーターをリセット
    HRESULT hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));

    // コマンドリストをリセット
    hr = commandList_->Reset(commandAllocator_.Get(), initialState);
    assert(SUCCEEDED(hr));

    isRecording_ = true;
}

void CommandList::Execute(bool waitForCompletion) {
    assert(isRecording_);

    // コマンドリストを閉じる
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    // コマンドリストを実行
    ID3D12CommandList* ppCommandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // フェンス値を進める
    fenceValue_++;

    // フェンス値をセット
    hr = commandQueue_->Signal(fence_.Get(), fenceValue_);
    assert(SUCCEEDED(hr));

    isRecording_ = false;

    // 必要に応じてGPU処理の完了を待機
    if (waitForCompletion) {
        WaitForGpu();
    }
}

void CommandList::WaitForGpu() {
    if (fence_->GetCompletedValue() < fenceValue_) {
        HRESULT hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
        assert(SUCCEEDED(hr));
        WaitForSingleObject(fenceEvent_, INFINITE);
    }
}

void CommandList::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = before;
    barrier.Transition.StateAfter = after;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    commandList_->ResourceBarrier(1, &barrier);
}