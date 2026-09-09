#include <Features/Model/Material/Material.h>
#include <Core/DXCommon/DXCommon.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/Debug.h>
#include <Features/Json/JsonSerializers.h>
#include <Utility/FileDialog/FileDialog.h>

#include <assimp/material.h>

#include <filesystem>
#include <fstream>

namespace Engine {

Material::Material(const Material& _other)
    : uvTransform_(_other.uvTransform_)
    , deffuseColor_(_other.deffuseColor_)
    , hasTexture_(_other.hasTexture_)
    , shiness_(_other.shiness_)
    , enableLighting_(_other.enableLighting_)
    , envScale_(_other.envScale_)
    , enableEnvironment_(_other.enableEnvironment_)
    , shadingStrength_(_other.shadingStrength_)
    , shadeColor_(_other.shadeColor_)
    , specularStrength_(_other.specularStrength_)
    , name_(_other.name_)
    , texturePath_(_other.texturePath_)
    , materialFilePath_(_other.materialFilePath_)
    , textureHandle_(_other.textureHandle_)
{
    // 新しいリソースを作成（ディープコピー）
    DXCommon* dxCommon = DXCommon::GetInstance();
    resorces_ = dxCommon->CreateBufferResource(sizeof(DataForGPU));
    resorces_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

    // データを転送
    TransferData();
}


void Material::Initialize(const std::string& _texturepath)
{
	DXCommon* dxCommon = DXCommon::GetInstance();

	resorces_ = dxCommon->CreateBufferResource(sizeof(DataForGPU));
	resorces_->Map(0, nullptr, reinterpret_cast<void**>(&constMap_));

	uvTransform_.SetOffset(Vector2(0.0f, 0.0f));
	uvTransform_.SetScale(Vector2(1.0f, 1.0f));
	uvTransform_.SetRotation(0.0f);


	shiness_ = 40.0f;
    shadingStrength_ = 1.0f;
    shadeColor_ = { 0.0f, 0.0f, 0.0f };
    specularStrength_ = 0.3f;

	enableLighting_ = true;

	texturePath_ = _texturepath;

	TransferData();
    LoadTexture();

}

void Material::LoadTexture()
{
	if (texturePath_ == "")
		textureHandle_ = 0;
	else
		textureHandle_ = TextureManager::GetInstance()->Load(texturePath_, "");
}

void Material::TransferData()
{
	Matrix4x4 affine = uvTransform_.GetMatrix();

	constMap_->uvTransform = affine;
	constMap_->deffuseColor = deffuseColor_;
	constMap_->shininess = shiness_;
	constMap_->enabledLighthig = enableLighting_;
    constMap_->hasTexture = hasTexture_ ? 1 : 0;
    constMap_->envScale = envScale_;
    constMap_->enableEnvironment = enableEnvironment_ ? 1 : 0;
    constMap_->specularStrength = specularStrength_;
    constMap_->shadingStrength = shadingStrength_;
    constMap_->shadeColor = shadeColor_;
}

void Material::MaterialQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index)
{
	TransferData();
    _commandList->SetGraphicsRootConstantBufferView(_index, resorces_->GetGPUVirtualAddress());
}

void Material::TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index) const
{
    _commandList->SetGraphicsRootDescriptorTable(_index, TextureManager::GetInstance()->GetGPUHandle(textureHandle_));
}

void Material::TextureQueueCommand(ID3D12GraphicsCommandList* _commandList, UINT _index, uint32_t _textureHandle) const
{
	_commandList->SetGraphicsRootDescriptorTable(_index, TextureManager::GetInstance()->GetGPUHandle(_textureHandle));
}

void Material::AnalyzeMaterial(const aiMaterial* _material)
{
    aiUVTransform uvTransform;
	if (_material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), uvTransform) == AI_SUCCESS)
	{
        uvTransform_.SetOffset(Vector2(uvTransform.mTranslation.x, uvTransform.mTranslation.y));
        uvTransform_.SetScale(Vector2(uvTransform.mScaling.x, uvTransform.mScaling.y));
        uvTransform_.SetRotation(uvTransform.mRotation);
    }
    else
    {
        uvTransform_.SetOffset(Vector2(0.0f, 0.0f));
        uvTransform_.SetScale(Vector2(1.0f, 1.0f));
        uvTransform_.SetRotation(0.0f);
    }

	aiColor3D meshColor;
	if (_material->Get(AI_MATKEY_COLOR_DIFFUSE, meshColor) == AI_SUCCESS)
	{
		deffuseColor_ = Vector4(meshColor.r, meshColor.g, meshColor.b, 1.0f);
	}
    else
    {
        deffuseColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // デフォルトの色
    }
    if (_material->Get(AI_MATKEY_SHININESS, shiness_) != AI_SUCCESS)
    {
        shiness_ = 40.0f; // デフォルトのシニアス値
    }
	if (_material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		hasTexture_ = true;
	else
		hasTexture_ = false;
}

bool Material::SaveToFile(const std::string& _filePath) const
{
    if (_filePath.empty())
        return false;

    std::filesystem::path filePath(_filePath);
    if (!filePath.has_extension())
        filePath.replace_extension(".json");

    try
    {
        if (filePath.has_parent_path())
            std::filesystem::create_directories(filePath.parent_path());

        const json materialJson = {
            {"version", 1},
            {"name", name_},
            {"texturePath", texturePath_},
            {"hasTexture", hasTexture_},
            {"diffuseColor", deffuseColor_},
            {"shininess", shiness_},
            {"enableLighting", enableLighting_},
            {"enableEnvironment", enableEnvironment_},
            {"environmentScale", envScale_},
            {"shadingStrength", shadingStrength_},
            {"shadeColor", shadeColor_},
            {"specularStrength", specularStrength_},
            {"uvTransform", {
                {"offset", uvTransform_.GetOffset()},
                {"scale", uvTransform_.GetScale()},
                {"rotation", uvTransform_.GetRotation()}
            }}
        };

        std::ofstream output(filePath);
        if (!output.is_open())
        {
            Debug::LogError("Material: failed to open save file: " + filePath.string() + "\n");
            return false;
        }

        output << materialJson.dump(4);
        if (!output.good())
        {
            Debug::LogError("Material: failed to write save file: " + filePath.string() + "\n");
            return false;
        }

        Debug::Log("Material saved: " + filePath.string() + "\n");
        return true;
    }
    catch (const std::exception& exception)
    {
        Debug::LogError("Material: save failed: " + std::string(exception.what()) + "\n");
        return false;
    }
}

bool Material::LoadFromFile(const std::string& _filePath)
{
    if (_filePath.empty())
        return false;

    try
    {
        std::ifstream input(_filePath);
        if (!input.is_open())
        {
            Debug::LogError("Material: failed to open load file: " + _filePath + "\n");
            return false;
        }

        json materialJson;
        input >> materialJson;
        if (!materialJson.is_object())
        {
            Debug::LogError("Material: JSON root must be an object: " + _filePath + "\n");
            return false;
        }

        // 欠けている項目は現在値を維持し、旧データにも前方互換で対応する。
        name_ = materialJson.value("name", name_);
        const std::string loadedTexturePath = materialJson.value("texturePath", texturePath_);
        hasTexture_ = materialJson.value("hasTexture", hasTexture_);
        deffuseColor_ = materialJson.value("diffuseColor", deffuseColor_);
        shiness_ = materialJson.value("shininess", shiness_);
        enableLighting_ = materialJson.value("enableLighting", enableLighting_);
        enableEnvironment_ = materialJson.value("enableEnvironment", enableEnvironment_);
        envScale_ = materialJson.value("environmentScale", envScale_);
        shadingStrength_ = materialJson.value("shadingStrength", shadingStrength_);
        shadeColor_ = materialJson.value("shadeColor", shadeColor_);
        specularStrength_ = materialJson.value("specularStrength", specularStrength_);

        if (materialJson.contains("uvTransform") && materialJson["uvTransform"].is_object())
        {
            const json& uvJson = materialJson["uvTransform"];
            uvTransform_.SetOffset(uvJson.value("offset", uvTransform_.GetOffset()));
            uvTransform_.SetScale(uvJson.value("scale", uvTransform_.GetScale()));
            uvTransform_.SetRotation(uvJson.value("rotation", uvTransform_.GetRotation()));
        }

        if (loadedTexturePath != texturePath_)
        {
            texturePath_ = loadedTexturePath;
            LoadTexture();
        }

        TransferData();
        Debug::Log("Material loaded: " + _filePath + "\n");
        return true;
    }
    catch (const std::exception& exception)
    {
        Debug::LogError("Material: load failed: " + std::string(exception.what()) + "\n");
        return false;
    }
}

void Material::Imgui()
{
#ifdef _DEBUG
    ImGui::PushID(this);

    ImGui::ColorEdit4("Diffuse Color", &deffuseColor_.x);
    ImGui::DragFloat("Shininess", &shiness_, 0.1f, 0.0f, 100.0f);
    ImGui::DragFloat("Shading Strength", &shadingStrength_, 0.01f, 0.0f, 1.0f);
    ImGui::ColorEdit3("Shade Color", &shadeColor_.x);
    ImGui::DragFloat("Specular", &specularStrength_, 0.01f, 0.0f, 1.0f);
    ImGui::Checkbox("Enable Lighting", &enableLighting_);
    ImGui::Checkbox("Enable Environment", &enableEnvironment_);
    ImGui::DragFloat("Environment Scale", &envScale_, 0.01f, 0.0f, 10.0f);

    ImGui::Text("Texture Path: %s", texturePath_.c_str());
    ImGui::SeparatorText("UV Transform");

    Vector2 offset = uvTransform_.GetOffset();
    Vector2 scale = uvTransform_.GetScale();
    float rotation = uvTransform_.GetRotation();

    ImGui::DragFloat2("UV Offset", &offset.x, 0.01f);
    ImGui::DragFloat2("UV Scale", &scale.x, 0.01f);
    ImGui::DragFloat("UV Rotation", &rotation, 0.01f);

    uvTransform_.SetOffset(offset);
    uvTransform_.SetScale(scale);
    uvTransform_.SetRotation(rotation);

    ImGui::Separator();

    const std::string jsonFilter = FileFilterBuilder()
        .AddCustom("Material JSON (*.json)", "*.json")
        .Build();

    if (!materialFilePath_.empty())
    {
        ImGui::TextWrapped("Material File: %s", materialFilePath_.c_str());
        if (ImGui::Button("Save Material"))
            SaveToFile(materialFilePath_);

        ImGui::SameLine();
        if (ImGui::Button("Reload Material"))
            LoadFromFile(materialFilePath_);
    }

    if (ImGui::Button("Save Material As..."))
    {
        const std::string defaultName = name_.empty() ? "Material.json" : name_ + ".json";
        const std::string filePath = FileDialog::SaveFileAs(jsonFilter, defaultName);
        if (!filePath.empty())
            SaveToFile(filePath);
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Material..."))
    {
        const std::string filePath = FileDialog::OpenFile(jsonFilter);
        if (!filePath.empty())
            LoadFromFile(filePath);
    }

    ImGui::PopID();
#endif // _DEBUG
}

} // namespace Engine
