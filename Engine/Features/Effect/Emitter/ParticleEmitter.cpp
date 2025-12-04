#include "ParticleEmitter.h"
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImguITools.h>
#include <algorithm>
#include <cstring>

#ifdef _DEBUG
// 静的メンバの定義
int ParticleEmitter::s_nextID_ = 0;
#endif

bool ParticleEmitter::Initialize(const std::string& _name)
{
    if (_name.empty())
    {
        SetError("Emitter name cannot be empty");
        return false;
    }

    ClearError();
    name_ = _name;
    isActive_ = false;
    isAlive_ = true;
    elapsedTime_ = 0.0f;

    try
    {
        InitJsonBinder();

#ifdef _DEBUG
        // 一意なIDを割り当て
        instanceID_ = s_nextID_++;

        // 名前をbufにコピー（安全に）
        strncpy_s(nameBuf_, sizeof(nameBuf_), name_.c_str(), _TRUNCATE);
#endif

        return true;
    }
    catch (const std::exception& e)
    {
        SetError("Initialization failed: " + std::string(e.what()));
        return false;
    }
}

void ParticleEmitter::Update(float _deltaTime)
{
    if (!isActive_ || !isAlive_) return;

    // エミッターの時間を更新
    elapsedTime_ += _deltaTime;

    // 遅延時間を超えてないとき
    if (elapsedTime_ < delayTime_) return;

    // 時間ごとにパーティクルを発生させる
    if (elapsedTime_ >= delayTime_)
    {
        // ループしているとき
        if (isLoop_)
        {
            // 一定時間ごとにパーティクルを発生させる
            if (elapsedTime_ >= delayTime_ + lifeTime)
            {
                elapsedTime_ = 0.0f;
            }
        }
        else
        {
            // ループしていないとき
            if (elapsedTime_ >= delayTime_ + lifeTime)
            {
                isActive_ = false;
                isAlive_ = false;
                return;
            }
        }
        GenerateParticles();
    }
}

void ParticleEmitter::Reset()
{
    elapsedTime_ = 0.0f;
    isActive_ = false;
    isAlive_ = true;
}

void ParticleEmitter::GenerateParticles()
{
    if (!ValidateSettings()) return;

    std::vector<std::unique_ptr<Particle>> particles;

    Quaternion q = Quaternion::EulerToQuaternion(rotationEuler_);
    Matrix4x4 emitterTransform = MakeAffineMatrix(
        Vector3(1.0f, 1.0f, 1.0f), // スケール
        q, // 回転
        position_ // 平行移動
    );

    Quaternion parentRotation = q;
    if (parentTransform_)
    {
        // 親のワールドトランスフォームを考慮
        emitterTransform *= parentTransform_->matWorld_;
        parentRotation = parentRotation * parentTransform_->quaternion_;
    }

    for (uint32_t i = 0; i < emitCount_; ++i)
    {
        ParticleInitParam initParam;

        initParam.isBillboard = initParams_.billboard;

        initParam.color;
        Vector3 rgb = initParams_.colorRGB.GetValue();
        float alpha = initParams_.colorA.GetValue();

        initParam.color = Vector4(rgb.x, rgb.y, rgb.z, alpha);

        initParam.isInfiniteLife = initParams_.lifeTimeType == ParticleLifeTimeType::Infinite;

        if (!initParam.isInfiniteLife)
        {
            initParam.lifeTime = initParams_.lifeTime.GetValue();
        }

        Vector3 emitterWorldPos = position_;

        switch (shape_)
        {
        case EmitterShape::Box:
        {
            Vector3 randomPos = RandomGenerator::GetInstance()->GetRandValue({ 0,0,0 }, boxSize_);
            randomPos -= boxSize_ / 2.0f; // 中心を基準にする
            Vector3 inner = initParams_.boxInnerSize.GetValue();

            // X軸: 内径より内側なら外側に押し出す
            if (std::abs(randomPos.x) < inner.x)
                randomPos.x = (randomPos.x < 0) ? -inner.x : inner.x;

            // Y軸: 内径より内側なら外側に押し出す
            if (std::abs(randomPos.y) < inner.y)
                randomPos.y = (randomPos.y < 0) ? -inner.y : inner.y;

            // Z軸: 内径より内側なら外側に押し出す
            if (std::abs(randomPos.z) < inner.z)
                randomPos.z = (randomPos.z < 0) ? -inner.z : inner.z;

            initParam.position = randomPos;

            break;
        }
        case EmitterShape::Sphere:
        {
            float sphereOffset = RandomGenerator::GetInstance()->GetRandValue(initParams_.sphereOffset.GetValue(), sphereRadius_);

            float rad = RandomGenerator::GetInstance()->GetUniformAngle();
            initParam.position.x = std::cosf(rad) * sphereOffset;
            initParam.position.y = RandomGenerator::GetInstance()->GetRandValue(-sphereOffset, sphereOffset);
            initParam.position.z = std::sinf(rad) * sphereOffset;
            break;
        }
        default:
            break;
        }
        initParam.position = Transform(initParam.position, emitterTransform);

        if (initParams_.directionType == ParticleDirectionType::Random ||
            initParams_.directionType == ParticleDirectionType::Fixed)
        {
            initParam.direction = initParams_.direction.GetValue();
        }
        else if (initParams_.directionType == ParticleDirectionType::Outward)
        {
            Vector3 dir = initParam.position - emitterWorldPos;
            dir = dir.Normalize();

            initParam.direction = dir;
        }
        else if (initParams_.directionType == ParticleDirectionType::Inward)
        {

            Vector3 dir = initParam.position - emitterWorldPos;
            dir = -dir.Normalize();

            initParam.direction = dir;
        }
        initParam.direction = TransformNormal(initParam.direction, emitterTransform);

        // 回転設定を追加 eulerはベクトルではないのでTransformNormalは不適切 quaternionなどに変換して親の回転を反映させる必要がある
        initParam.rotate = Vector3::QuaternionToEuler(Quaternion::EulerToQuaternion(initParams_.rotation.GetValue()) * parentRotation);
        initParam.rotationSpeed = initParams_.rotationSpeed.GetValue();
        initParam.size = initParams_.size.GetValue();
        initParam.speed = initParams_.speed.GetValue();

        auto p = std::make_unique<Particle>();
        particles[i] = std::move(p);
        particles[i]->Initialize(initParam);
    }

    ParticleRenderSettings settings;
    settings.blendMode = blendMode_;
    settings.cullBack = cullBack_;

    if (useModelName_ == "")
        useModelName_ = "cube/cube.obj";

    uint32_t textureHandle = TextureManager::GetInstance()->Load(initParams_.textureName);

    // groupnameには仮でエミッターの名前を入れている
    ParticleSystem::GetInstance()->AddParticles(name_, useModelName_, std::move(particles), settings, textureHandle, initParams_.modifiers);

}

void ParticleEmitter::EmitSingle()
{
    uint32_t originalCount = emitCount_;
    emitCount_ = 1;
    GenerateParticles();
    emitCount_ = originalCount;
}

void ParticleEmitter::EmitBurst(uint32_t _count)
{
    uint32_t originalCount = emitCount_;
    emitCount_ = _count;
    GenerateParticles();
    emitCount_ = originalCount;
}

void ParticleEmitter::SetName(const std::string& _name)
{
    if (!_name.empty())
    {
        name_ = _name;
        ClearError();
#ifdef _DEBUG
        strncpy_s(nameBuf_, sizeof(nameBuf_), name_.c_str(), _TRUNCATE);
#endif
    }
    else
    {
        SetError("Name cannot be empty");
    }
}

void ParticleEmitter::SetModelName(const std::string& _modelName)
{
    useModelName_ = _modelName;
    ClearError();
}

void ParticleEmitter::SetTextureName(const std::string& _textureName)
{
    initParams_.textureName = _textureName;
    ClearError();
}

void ParticleEmitter::SetTimeChannel(const std::string& _channel)
{
    timeChannel_ = _channel;
}

bool ParticleEmitter::HasModifier(std::string_view _modifierName) const
{
    return std::find_if(initParams_.modifiers.begin(), initParams_.modifiers.end(),
        [_modifierName](const std::string& _modifier) {
            return _modifier == _modifierName;
        }) != initParams_.modifiers.end();
}

bool ParticleEmitter::SaveToFile() const
{
    try
    {
        if (jsonBinder_)
        {
            jsonBinder_->Save();
            ClearError();
            return true;
        }
        else
        {
            SetError("JsonBinder not initialized");
            return false;
        }
    }
    catch (const std::exception& e)
    {
        SetError("Failed to save to file: " + std::string(e.what()));
        return false;
    }
}

void ParticleEmitter::LoadTexture(std::string_view _texturePath)
{
    try
    {
        TextureManager::GetInstance()->Load(std::string(_texturePath));
        initParams_.textureName = _texturePath;
        ClearError();
    }
    catch (const std::exception& e)
    {
        SetError("Failed to load texture: " + std::string(e.what()));
    }
}

void ParticleEmitter::LoadModel(std::string_view _modelPath)
{
    try
    {
        Model* model = Model::CreateFromFile(std::string(_modelPath));
        if (model)
        {
            useModelName_ = _modelPath;
            ClearError();
        }
        else
        {
            SetError("Failed to create model from path");
        }
    }
    catch (const std::exception& e)
    {
        SetError("Failed to load model: " + std::string(e.what()));
    }
}

void ParticleEmitter::AddModifier(const std::string& _modifierName)
{
    if (!_modifierName.empty() && !HasModifier(_modifierName))
    {
        initParams_.modifiers.push_back(_modifierName);
        ClearError();
    }
    else if (_modifierName.empty())
    {
        SetError("Modifier name cannot be empty");
    }
    else
    {
        SetError("Modifier already exists");
    }
}

bool ParticleEmitter::RemoveModifier(std::string_view _modifierName)
{
    auto it = std::find_if(initParams_.modifiers.begin(), initParams_.modifiers.end(),
        [_modifierName](const std::string& _modifier) {
            return _modifier == _modifierName;
        });

    if (it != initParams_.modifiers.end())
    {
        initParams_.modifiers.erase(it);
        ClearError();
        return true;
    }
    else
    {
        SetError("Modifier not found");
        return false;
    }
}

void ParticleEmitter::InitJsonBinder()
{
    jsonBinder_ = std::make_unique<JsonBinder>(name_, "Resources/data/Effect/Emitters/");

    jsonBinder_->RegisterVariable("offset", &offset_);
    jsonBinder_->RegisterVariable("shape", reinterpret_cast<uint32_t*>(&shape_));
    jsonBinder_->RegisterVariable("emitPerSecond", &emitPerSecond_);
    jsonBinder_->RegisterVariable("emitCount", &emitCount_);

    jsonBinder_->RegisterVariable("isLoop", &isLoop_);
    jsonBinder_->RegisterVariable("delayTime", &delayTime_);
    jsonBinder_->RegisterVariable("lifeTime", &lifeTime);

    jsonBinder_->RegisterVariable("position", &position_);
    jsonBinder_->RegisterVariable("rotation", &rotation_); // TODO
    jsonBinder_->RegisterVariable("boxSize", &boxSize_);
    jsonBinder_->RegisterVariable("sphereRadius", &sphereRadius_);

    jsonBinder_->RegisterVariable("useModelName", &useModelName_);


    //InitParams

    jsonBinder_->RegisterVariable("billboard_x", &initParams_.billboard[0]);
    jsonBinder_->RegisterVariable("billboard_y", &initParams_.billboard[1]);
    jsonBinder_->RegisterVariable("billboard_z", &initParams_.billboard[2]);

    jsonBinder_->RegisterVariable("BoxOffset_Random", &initParams_.boxInnerSize.isRandom);
    jsonBinder_->RegisterVariable("BoxOffset_Min", &initParams_.boxInnerSize.minV);
    jsonBinder_->RegisterVariable("BoxOffset_Max", &initParams_.boxInnerSize.maxV);
    jsonBinder_->RegisterVariable("BoxOffset_Value", &initParams_.boxInnerSize.value);

    jsonBinder_->RegisterVariable("SphereOffset_Random", &initParams_.sphereOffset.isRandom);
    jsonBinder_->RegisterVariable("SphereOffset_Min", &initParams_.sphereOffset.minV);
    jsonBinder_->RegisterVariable("SphereOffset_Max", &initParams_.sphereOffset.maxV);
    jsonBinder_->RegisterVariable("SphereOffset_Value", &initParams_.sphereOffset.value);

    jsonBinder_->RegisterVariable("direction_Type", reinterpret_cast<uint32_t*>(&initParams_.directionType));
    jsonBinder_->RegisterVariable("direction_Min", &initParams_.direction.minV);
    jsonBinder_->RegisterVariable("direction_Max", &initParams_.direction.maxV);
    jsonBinder_->RegisterVariable("direction_Value", &initParams_.direction.value);
    jsonBinder_->RegisterVariable("direction_Random", &initParams_.direction.isRandom);

    jsonBinder_->RegisterVariable("speed_Random", &initParams_.speed.isRandom);
    jsonBinder_->RegisterVariable("speed_Min", &initParams_.speed.minV);
    jsonBinder_->RegisterVariable("speed_Max", &initParams_.speed.maxV);
    jsonBinder_->RegisterVariable("speed_Value", &initParams_.speed.value);

    jsonBinder_->RegisterVariable("deceleration_Random", &initParams_.deceleration.isRandom);
    jsonBinder_->RegisterVariable("deceleration_Min", &initParams_.deceleration.minV);
    jsonBinder_->RegisterVariable("deceleration_Max", &initParams_.deceleration.maxV);
    jsonBinder_->RegisterVariable("deceleration_Value", &initParams_.deceleration.value);

    jsonBinder_->RegisterVariable("lifeTime_Type", reinterpret_cast<uint32_t*>(&initParams_.lifeTimeType));
    jsonBinder_->RegisterVariable("lifeTime_Min", &initParams_.lifeTime.minV);
    jsonBinder_->RegisterVariable("lifeTime_Max", &initParams_.lifeTime.maxV);
    jsonBinder_->RegisterVariable("lifeTime_Value", &initParams_.lifeTime.value);
    jsonBinder_->RegisterVariable("lifeTime_Random", &initParams_.lifeTime.isRandom);

    jsonBinder_->RegisterVariable("size_Random", &initParams_.size.isRandom);
    jsonBinder_->RegisterVariable("size_Min", &initParams_.size.minV);
    jsonBinder_->RegisterVariable("size_Max", &initParams_.size.maxV);
    jsonBinder_->RegisterVariable("size_Value", &initParams_.size.value);

    jsonBinder_->RegisterVariable("rotation_Random", &initParams_.rotation.isRandom);
    jsonBinder_->RegisterVariable("rotation_Min", &initParams_.rotation.minV);
    jsonBinder_->RegisterVariable("rotation_Max", &initParams_.rotation.maxV);
    jsonBinder_->RegisterVariable("rotation_Value", &initParams_.rotation.value);

    jsonBinder_->RegisterVariable("rotationSpeed_Random", &initParams_.rotationSpeed.isRandom);
    jsonBinder_->RegisterVariable("rotationSpeed_Min", &initParams_.rotationSpeed.minV);
    jsonBinder_->RegisterVariable("rotationSpeed_Max", &initParams_.rotationSpeed.maxV);
    jsonBinder_->RegisterVariable("rotationSpeed_Value", &initParams_.rotationSpeed.value);

    jsonBinder_->RegisterVariable("RGB_Random", &initParams_.colorRGB.isRandom);
    jsonBinder_->RegisterVariable("RGB_Min", &initParams_.colorRGB.minV);
    jsonBinder_->RegisterVariable("RGB_Max", &initParams_.colorRGB.maxV);
    jsonBinder_->RegisterVariable("RGB_Value", &initParams_.colorRGB.value);

    jsonBinder_->RegisterVariable("alpha_Random", &initParams_.colorA.isRandom);
    jsonBinder_->RegisterVariable("alpha_Min", &initParams_.colorA.minV);
    jsonBinder_->RegisterVariable("alpha_Max", &initParams_.colorA.maxV);
    jsonBinder_->RegisterVariable("alpha_Value", &initParams_.colorA.value);

    jsonBinder_->RegisterVariable("texturePath", &initParams_.textureName);

    jsonBinder_->RegisterVariable("modifiers", &initParams_.modifiers);

    jsonBinder_->RegisterVariable("cullBack", &cullBack_);
    jsonBinder_->RegisterVariable("blendMode", reinterpret_cast<uint32_t*>(&blendMode_));

}

bool ParticleEmitter::ValidateSettings() const
{
    if (name_.empty())
    {
        SetError("Emitter name is empty");
        return false;
    }

    if (emitCount_ == 0)
    {
        SetError("Emit count must be greater than 0");
        return false;
    }

    if (shape_ >= EmitterShape::Count)
    {
        SetError("Invalid emitter shape");
        return false;
    }

    ClearError();
    return true;
}


#ifdef _DEBUG
void ParticleEmitter::ShowDebugWindow()
{
    // 一意なIDを使用してウィンドウを識別
    ImGui::PushID(this);
    {
        ImGui::PushID("emitter");
        if (ImGui::Button("Emit"))
            GenerateParticles();

        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            if (jsonBinder_)
                jsonBinder_->Save();
        }

        if (ImGui::CollapsingHeader("Emitter Settings"))
        {
            ImGui::Text("Emitter Name");

            // 読み取り専用の名前表示
            ImGui::BeginDisabled();
            ImGui::InputText("##EmitterName", nameBuf_, sizeof(nameBuf_));
            ImGui::EndDisabled();

            ImGui::Separator();

            if (ImGui::TreeNode("Emitter Shape"))
            {
                // 安全なenum処理
                int currentShape = static_cast<int>(shape_);

                if (ImGui::RadioButton("Box", &currentShape, static_cast<int>(EmitterShape::Box)))
                    shape_ = EmitterShape::Box;

                if (ImGui::RadioButton("Sphere", &currentShape, static_cast<int>(EmitterShape::Sphere)))
                    shape_ = EmitterShape::Sphere;

                ImGui::TreePop();

                switch (shape_)
                {
                case EmitterShape::Box:
                    ImGui::DragFloat3("Box size", &boxSize_.x, 0.01f, 0.01f, 100.0f);
                    break;
                case EmitterShape::Sphere:
                    ImGui::DragFloat("Sphere radius", &sphereRadius_, 0.01f, 0.01f, 100.0f);
                    break;
                default:
                    break;
                }
            }

            if (ImGui::TreeNode("Position"))
            {
                ImGui::DragFloat3("Position", &position_.x, 0.01f);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Emit Settings"))
            {
                // 安全な整数入力
                int emitCount = static_cast<int>(emitCount_);
                if (ImGui::InputInt("Emit count", &emitCount))
                {
                    emitCount_ = static_cast<uint32_t>(std::max(1, emitCount));
                }

                int emitPerSecond = static_cast<int>(emitPerSecond_);
                if (ImGui::InputInt("Emit per second", &emitPerSecond))
                {
                    emitPerSecond_ = static_cast<uint32_t>(std::max(1, emitPerSecond));
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Time"))
            {
                ImGui::DragFloat("Delay Time", &delayTime_, 0.01f, 0.0f, 100.0f);
                ImGui::DragFloat("Life Time", &lifeTime, 0.01f, 0.0f, 100.0f);
                ImGui::Checkbox("Loop", &isLoop_);

                ImGui::TreePop();
            }

            if (parentTransform_ != nullptr)
            {
                if (ImGui::TreeNode("Emitter Offset"))
                {
                    ImGui::DragFloat3("Offset", &offset_.x, 0.01f);
                    ImGui::TreePop();
                }
            }
        }
        ImGui::PopID();

        ImGui::PushID("initParams");
        if (ImGui::CollapsingHeader("Particle Init Params"))
        {
            if (ImGui::TreeNode("Use Model"))
            {
                ImGui::Text("ModelPath");
                ImGui::InputText("##ModelPath", modelPath_, sizeof(modelPath_));

                if (ImGui::Button("Apply##ModelPath"))
                {
                    LoadModel(modelPath_);
                    memset(modelName_, 0, sizeof(modelName_));
                }

                ImGui::Separator();
                ImGui::Text("ModelName");
                ImGui::InputText("##ModelName", modelName_, sizeof(modelName_));

                if (ImGui::Button("Apply##ModelName"))
                {
                    Model* model = ModelManager::GetInstance()->FindSameModel(modelName_);
                    if (model != nullptr)
                    {
                        useModelName_ = modelName_;
                        memset(modelPath_, 0, sizeof(modelPath_));
                    }
                    else
                    {
                        ImGui::Text("Error: Model not found");
                    }
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Use Texture"))
            {
                ImGui::Text("TextureRoot");
                ImGui::InputText("##TextureRoot", textureRoot_, sizeof(textureRoot_));

                ImGui::Text("TexturePath");
                ImGui::InputText("##TexturePath", texturePath_, sizeof(texturePath_));

                if (ImGui::Button("Apply##TexturePath"))
                {
                    LoadTexture(texturePath_);
                }
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Billboard"))
            {
                ImGui::Text("Billboard");

                // 安全な配列アクセス
                if (initParams_.billboard.size() >= 3)
                {
                    ImGui::Checkbox("X", &initParams_.billboard[0]);
                    ImGui::SameLine();
                    ImGui::Checkbox("Y", &initParams_.billboard[1]);
                    ImGui::SameLine();
                    ImGui::Checkbox("Z", &initParams_.billboard[2]);
                }
                ImGui::TreePop();
            }

            // RenderSetting
            if (ImGui::TreeNode("Render Setting"))
            {
                ImGui::Text("Blend Mode");

                // 安全なenum処理
                int currentBlendMode = static_cast<int>(blendMode_);

                if (ImGui::RadioButton("Normal", &currentBlendMode, static_cast<int>(PSOFlags::BlendMode::Normal)))
                    blendMode_ = PSOFlags::BlendMode::Normal;
                if (ImGui::RadioButton("Add", &currentBlendMode, static_cast<int>(PSOFlags::BlendMode::Add)))
                    blendMode_ = PSOFlags::BlendMode::Add;
                if (ImGui::RadioButton("Sub", &currentBlendMode, static_cast<int>(PSOFlags::BlendMode::Sub)))
                    blendMode_ = PSOFlags::BlendMode::Sub;
                if (ImGui::RadioButton("Mul", &currentBlendMode, static_cast<int>(PSOFlags::BlendMode::Multiply)))
                    blendMode_ = PSOFlags::BlendMode::Multiply;
                if (ImGui::RadioButton("Screen", &currentBlendMode, static_cast<int>(PSOFlags::BlendMode::Screen)))
                    blendMode_ = PSOFlags::BlendMode::Screen;

                ImGui::Separator();
                ImGui::Checkbox("Cull Back", &cullBack_);

                ImGui::TreePop();
            }

            // 各設定関数の呼び出し
            if (ImGui::TreeNode("Life Time"))
            {
                DebugWindowForLifeTime();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Position"))
            {
                DebugWindowForPosition();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Size"))
            {
                DebugWindowForSize();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Direction"))
            {
                DebugWindowForDirection();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Speed"))
            {
                DebugWindowForSpeed();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Rotation"))
            {
                DebugWindowForRotation();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Deceleration"))
            {
                DebugWindowForDeceleration();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Color"))
            {
                DebugWindowForColor();
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Modifiers"))
            {
                DebugWindowForModifier();
                ImGui::TreePop();
            }
        }
        ImGui::PopID();
    }
    ImGui::PopID(); // Pop ID for this emitter
}

// デバッグウィンドウ関数の実装
void ParticleEmitter::DebugWindowForSize()
{
    auto& sizeParams = initParams_.size;

    int imSize = static_cast<int>(!sizeParams.isRandom);

    ImGui::RadioButton("Random", &imSize, 0);
    ImGui::RadioButton("Fixed", &imSize, 1);

    sizeParams.isRandom = (imSize == 0);

    if (sizeParams.isRandom)
    {
        ImGui::DragFloat3("Min", &sizeParams.minV.x, 0.01f);
        ImGui::DragFloat3("Max", &sizeParams.maxV.x, 0.01f);
    }
    else
    {
        ImGui::DragFloat3("Size", &sizeParams.value.x, 0.01f);
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForPosition()
{
    switch (shape_)
    {
    case EmitterShape::Box:
    {
        auto& boxOffset = initParams_.boxInnerSize;

        int imBoxOffset = static_cast<int>(!boxOffset.isRandom);

        ImGui::RadioButton("Random", &imBoxOffset, 0);
        ImGui::RadioButton("Fixed", &imBoxOffset, 1);

        boxOffset.isRandom = (imBoxOffset == 0);

        if (boxOffset.isRandom)
        {
            ImGui::DragFloat3("Min", &boxOffset.minV.x, 0.01f);
            ImGui::DragFloat3("Max", &boxOffset.maxV.x, 0.01f);
        }
        else
        {
            ImGui::DragFloat3("Box Inner Size", &boxOffset.value.x, 0.01f);
        }
        break;
    }
    case EmitterShape::Sphere:
    {
        auto& sphereOffset = initParams_.sphereOffset;
        int imSphereOffset = static_cast<int>(!sphereOffset.isRandom);

        ImGui::RadioButton("Random", &imSphereOffset, 0);
        ImGui::RadioButton("Fixed", &imSphereOffset, 1);

        sphereOffset.isRandom = (imSphereOffset == 0);

        if (sphereOffset.isRandom)
        {
            ImGui::DragFloat("Min", &sphereOffset.minV, 0.01f);
            ImGui::DragFloat("Max", &sphereOffset.maxV, 0.01f);
        }
        else
        {
            ImGui::DragFloat("Sphere Inner Size", &sphereOffset.value, 0.01f);
        }
        break;
    }
    default:
        break;
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForDirection()
{
    int currentDirectionType = static_cast<int>(initParams_.directionType);

    if (ImGui::RadioButton("Outward", &currentDirectionType, static_cast<int>(ParticleDirectionType::Outward)))
        initParams_.directionType = ParticleDirectionType::Outward;
    if (ImGui::RadioButton("Inward", &currentDirectionType, static_cast<int>(ParticleDirectionType::Inward)))
        initParams_.directionType = ParticleDirectionType::Inward;
    if (ImGui::RadioButton("Random", &currentDirectionType, static_cast<int>(ParticleDirectionType::Random)))
        initParams_.directionType = ParticleDirectionType::Random;
    if (ImGui::RadioButton("Fixed", &currentDirectionType, static_cast<int>(ParticleDirectionType::Fixed)))
        initParams_.directionType = ParticleDirectionType::Fixed;

    if (initParams_.directionType == ParticleDirectionType::Random)
    {
        initParams_.direction.isRandom = true;
        ImGui::DragFloat3("Min", &initParams_.direction.minV.x, 0.01f);
        ImGui::DragFloat3("Max", &initParams_.direction.maxV.x, 0.01f);
    }
    else if (initParams_.directionType == ParticleDirectionType::Fixed)
    {
        initParams_.direction.isRandom = false;
        ImGui::DragFloat3("Direction", &initParams_.direction.value.x, 0.01f);
    }
    else
    {
        initParams_.direction.isRandom = false;
        // Outward, Inwardの場合は指定できない
        ImGui::Text("Direction is fixed for Outward and Inward.");
        ImGui::BeginDisabled();
        ImGui::DragFloat3("Direction", &initParams_.direction.value.x, 0.01f);
        ImGui::EndDisabled();
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForSpeed()
{
    auto& speed = initParams_.speed;

    int imSpeed = speed.isRandom ? 0 : 1;
    ImGui::RadioButton("Random", &imSpeed, 0);
    ImGui::RadioButton("Fixed", &imSpeed, 1);

    speed.isRandom = (imSpeed == 0);

    if (speed.isRandom)
    {
        ImGui::DragFloat("Min", &speed.minV, 0.01f);
        ImGui::DragFloat("Max", &speed.maxV, 0.01f);
    }
    else
    {
        ImGui::DragFloat("Speed", &speed.value, 0.01f);
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForRotation()
{
    auto& rotation = initParams_.rotation;
    int imRotation = static_cast<int>(!rotation.isRandom);

    ImGui::Separator();

    ImGui::RadioButton("Random##rot", &imRotation, 0);
    ImGui::RadioButton("Fixed##rot", &imRotation, 1);

    rotation.isRandom = (imRotation == 0);

    if (rotation.isRandom)
    {
        ImGui::DragFloat3("Min##rot", &rotation.minV.x, 0.01f);
        ImGui::DragFloat3("Max##rot", &rotation.maxV.x, 0.01f);
    }
    else
    {
        ImGui::DragFloat3("Rotation##rot", &rotation.value.x, 0.01f);
    }

    ImGui::SeparatorText("Rotation Speed");

    auto& rotationSpeed = initParams_.rotationSpeed;
    int imRotationSpeed = static_cast<int>(!rotationSpeed.isRandom);

    ImGui::RadioButton("Random##rotSpeed", &imRotationSpeed, 0);
    ImGui::RadioButton("Fixed##rotSpeed", &imRotationSpeed, 1);

    rotationSpeed.isRandom = (imRotationSpeed == 0);

    if (rotationSpeed.isRandom)
    {
        ImGui::DragFloat3("Min##rotSpeed", &rotationSpeed.minV.x, 0.01f);
        ImGui::DragFloat3("Max##rotSpeed", &rotationSpeed.maxV.x, 0.01f);
    }
    else
    {
        ImGui::DragFloat3("Rotation Speed##rotSpeed", &rotationSpeed.value.x, 0.01f);
    }

    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForDeceleration()
{
    auto& deceleration = initParams_.deceleration;
    int imDece = static_cast<int>(!deceleration.isRandom);
    ImGui::RadioButton("Random", &imDece, 0);
    ImGui::RadioButton("Fixed", &imDece, 1);

    deceleration.isRandom = (imDece == 0);

    if (deceleration.isRandom)
    {
        ImGui::DragFloat("Min", &deceleration.minV, 0.01f);
        ImGui::DragFloat("Max", &deceleration.maxV, 0.01f);
    }
    else
    {
        ImGui::DragFloat("Deceleration", &deceleration.value, 0.01f);
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForLifeTime()
{
    int currentLifeTimeType = static_cast<int>(initParams_.lifeTimeType);

    if (ImGui::RadioButton("Infinite", &currentLifeTimeType, static_cast<int>(ParticleLifeTimeType::Infinite)))
        initParams_.lifeTimeType = ParticleLifeTimeType::Infinite;
    if (ImGui::RadioButton("Random", &currentLifeTimeType, static_cast<int>(ParticleLifeTimeType::Random)))
        initParams_.lifeTimeType = ParticleLifeTimeType::Random;
    if (ImGui::RadioButton("Fixed", &currentLifeTimeType, static_cast<int>(ParticleLifeTimeType::Fixed)))
        initParams_.lifeTimeType = ParticleLifeTimeType::Fixed;

    if (initParams_.lifeTimeType == ParticleLifeTimeType::Random)
    {
        initParams_.lifeTime.isRandom = true;
        ImGui::DragFloat("Min", &initParams_.lifeTime.minV, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Max", &initParams_.lifeTime.maxV, 0.01f, 0.0f, 100.0f);
    }
    else if (initParams_.lifeTimeType == ParticleLifeTimeType::Fixed)
    {
        initParams_.lifeTime.isRandom = false;
        ImGui::DragFloat("Life Time", &initParams_.lifeTime.value, 0.01f, 0.0f, 100.0f);
    }
    else
    {
        // Infiniteの場合は指定できない
        ImGui::Text("Life Time is fixed for Infinite.");
        ImGui::BeginDisabled();
        ImGui::DragFloat("Life Time", &initParams_.lifeTime.value, 0.01f);
        ImGui::EndDisabled();
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForColor()
{
    if (ImGui::TreeNode("Color RGB"))
    {
        auto& color = initParams_.colorRGB;
        int colorPtr = color.isRandom ? 0 : 1;
        ImGui::RadioButton("Random", &colorPtr, 0);
        ImGui::RadioButton("Fixed", &colorPtr, 1);

        color.isRandom = (colorPtr == 0);

        if (color.isRandom)
        {
            ImGui::ColorEdit3("Min", &color.minV.x);
            ImGui::ColorEdit3("Max", &color.maxV.x);
        }
        else
        {
            ImGui::ColorEdit3("Color", &color.value.x);
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Color Alpha"))
    {
        auto& alpha = initParams_.colorA;

        int imAlpha = alpha.isRandom ? 0 : 1;

        ImGui::RadioButton("Random", &imAlpha, 0);
        ImGui::RadioButton("Fixed", &imAlpha, 1);

        alpha.isRandom = (imAlpha == 0);

        if (alpha.isRandom)
        {
            ImGui::DragFloat("Min", &alpha.minV, 0.01f);
            ImGui::DragFloat("Max", &alpha.maxV, 0.01f);
        }
        else
        {
            ImGui::DragFloat("Alpha", &alpha.value, 0.01f, 0.0f, 1.0f);
        }
        ImGui::TreePop();
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForModifier()
{
    ImGui::Text("Modifier Name");
    ImGui::InputText("##ModifierName", modifierName_, sizeof(modifierName_));

    if (ImGui::Button("Add Modifier"))
    {
        if (strlen(modifierName_) > 0)
        {
            AddModifier(std::string(modifierName_));
            memset(modifierName_, 0, sizeof(modifierName_));
        }
    }

    static int removeIndex = -1;
    ImGui::InputInt("Remove Index", &removeIndex);

    if (ImGui::Button("Remove Modifier"))
    {
        if (removeIndex >= 0 && removeIndex < static_cast<int>(initParams_.modifiers.size()))
        {
            auto it = initParams_.modifiers.begin();
            std::advance(it, removeIndex);
            RemoveModifier(*it);
            removeIndex = -1;
        }
    }

    ImGui::Separator();

    if (ImGui::TreeNode("Modifier List"))
    {
        for (size_t i = 0; i < initParams_.modifiers.size(); ++i)
        {
            ImGui::Text("[%zu] %s", i, initParams_.modifiers[i].c_str());
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
}

#endif // _DEBUG