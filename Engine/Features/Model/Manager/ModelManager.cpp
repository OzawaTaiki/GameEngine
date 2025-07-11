#include <Features/Model/Manager/ModelManager.h>

#include <Core/DXCommon/PSOManager/PSOManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <cassert>
#include <LevelEditorLoader.cpp>

ModelManager* ModelManager::GetInstance()
{
    static ModelManager instance;
    return &instance;
}

void ModelManager::Initialize()
{
    psoFlags_ = PSOFlags::ForNormalModel();
    psoFlagsForAlpha_ = PSOFlags::ForAlphaModel();


    /// PSOを取得
    auto pso = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlags_);
    // PSOが生成されているか確認
    assert(pso.has_value() && pso != nullptr);
    graphicsPipelineState_ = pso.value();

    /// RootSingnatureを取得
    auto rootSignature = PSOManager::GetInstance()->GetRootSignature(psoFlags_);
    // 生成されているか確認
    assert(rootSignature.has_value() && rootSignature != nullptr);
    rootSignature_ = rootSignature.value();

    auto psoAlpha = PSOManager::GetInstance()->GetPipeLineStateObject(psoFlagsForAlpha_);
    // PSOが生成されているか確認
    assert(psoAlpha.has_value() && psoAlpha != nullptr);
    graphicsPipelineStateForAlpha_ = psoAlpha.value();

    CreateComputePipeline();
 }

void ModelManager::PreDrawForObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineState_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ModelManager::PreDrawForAlphaObjectModel() const
{
    auto commandList = DXCommon::GetInstance()->GetCommandList();

    commandList->SetGraphicsRootSignature(rootSignature_);
    commandList->SetPipelineState(graphicsPipelineStateForAlpha_);

    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

Model* ModelManager::FindSameModel(const std::string& _name)
{
    auto it = models_.find(_name);
    if (it != models_.end())
        return models_[_name].get();

    return nullptr;
}

Model* ModelManager::Create(const std::string& _name)
{
    std::string name = _name;

    auto it = models_.find(name);
    if (it != models_.end())
    {
        name = name + std::to_string(models_.size());
    }

    models_[name] = std::make_unique<Model>();
    return models_[name].get();
}

void ModelManager::CreateComputePipeline()
{
    HRESULT hr = S_FALSE;


    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRanges[4] = {};
    // gMatrixPalette
    descriptorRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[0].NumDescriptors = 1;
    descriptorRanges[0].BaseShaderRegister = 0;
    descriptorRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // inputVertex
    descriptorRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[1].NumDescriptors = 1;
    descriptorRanges[1].BaseShaderRegister = 1;
    descriptorRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // gInfluence
    descriptorRanges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRanges[2].NumDescriptors = 1;
    descriptorRanges[2].BaseShaderRegister = 2;
    descriptorRanges[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // outputVertex
    descriptorRanges[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    descriptorRanges[3].NumDescriptors = 1;
    descriptorRanges[3].BaseShaderRegister = 0;
    descriptorRanges[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


    D3D12_ROOT_PARAMETER rootParameters[5] = {};

    // gMatrixPalette
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRanges[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // inputVertex
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[1].DescriptorTable.pDescriptorRanges = &descriptorRanges[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // gInfluence
    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRanges[2];
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // outputVertex
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[3].DescriptorTable.pDescriptorRanges = &descriptorRanges[3];
    rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;

    // skinnedInformation
    rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    rootParameters[4].Descriptor.ShaderRegister = 0;


    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);


    //シリアライズしてバイナリする
    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    if (FAILED(hr))
    {
        Debug::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
        assert(false);
    }
    hr = DXCommon::GetInstance()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignatureForCompute_));
    assert(SUCCEEDED(hr));


    // ComputeShaderの設定
    Microsoft::WRL::ComPtr<IDxcBlob> computeShaderBlob = nullptr;
    // ComputeShaderのコンパイル
    computeShaderBlob = PSOManager::GetInstance()->ComplieShader(L"Skinning.CS.hlsl", L"cs_6_0");
    assert(computeShaderBlob != nullptr);

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = rootSignatureForCompute_.Get();
    computePsoDesc.CS = { computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize() };

    hr = DXCommon::GetInstance()->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipeline_));
    assert(SUCCEEDED(hr));

}
