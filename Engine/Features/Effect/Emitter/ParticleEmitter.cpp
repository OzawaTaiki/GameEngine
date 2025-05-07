#include "ParticleEmitter.h"
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Debug/ImguITools.h>

void ParticleEmitter::Initialize(const std::string& _name)
{
    name_ = _name;
    isActive_ = false;

    InitJsonBinder();

#ifdef _DEBUG
    // 名前をbufにコピー
    strcpy_s(nameBuf_, 256, name_.c_str());

#endif // _DEBUG


}
void ParticleEmitter::Update(float _deltaTime)
{
    // 有効じゃないとき
    if (!isActive_)         return;

    // エミッターの時間を更新
    elapsedTime_ += _deltaTime;

    // 遅延時間を超えてないとき
    if (elapsedTime_ < delayTime_)        return;

    // 有効になる
    isActive_ = true;

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
                return;
            }
        }
        GenerateParticles();
    }
}

void ParticleEmitter::ShowDebugWindow()
{
#ifdef _DEBUG


    ImGui::Separator();
    ImGui::BeginTabBar("Emitter");
    {
        if(ImGui::BeginTabItem(name_.c_str()))
        {

            if (ImGui::Button("Emit"))      GenerateParticles();
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                jsonBinder_->Save();
            }

            if (ImGui::CollapsingHeader("Emitter Settings"))
            {
                ImGui::Text("Emitter Name");

                ImGui::BeginDisabled(1);
                ImGui::InputText("##EmitterName", name_.data(), name_.size());
                ImGui::EndDisabled();

                ImGui::Separator();
                if (ImGui::TreeNode("Emitter Shape"))
                {
                    int* shapePtr = reinterpret_cast<int*>(&shape_);

                    ImGui::RadioButton("Box", shapePtr, static_cast<int>(EmitterShape::Box));
                    ImGui::RadioButton("Sphere", shapePtr, static_cast<int>(EmitterShape::Sphere));
                    //ImGui::RadioButton("Cone", &shape_, static_cast<int>(EmitterShape::Cone));
                    //ImGui::RadioButton("Cylinder", &shape_, static_cast<int>(EmitterShape::Cylinder));
                    //ImGui::RadioButton("Plane", &shape_, static_cast<int>(EmitterShape::Plane));
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
                if (ImGui::TreeNode("Emit Settings"))
                {
                    ImGui::InputInt("Emit count", reinterpret_cast<int*>(&emitCount_));
                    ImGui::InputInt("Emit per second", reinterpret_cast<int*>(&emitPerSecond_));

                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Time"))
                {
                    ImGui::DragFloat("Delay Time", &delayTime_, 0.01f);
                    ImGui::DragFloat("Life Time", &lifeTime, 0.01f);
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

            if (ImGui::CollapsingHeader("Particle Init Params"))
            {
                if (ImGui::TreeNode("Use Model"))
                {
                    ImGui::Text("ModelPath");
                    ImGui::InputText("##ModelPath", modelPath_, 256);
                    if (ImGui::Button("Apply##ModelPath"))
                    {
                        Model* model = Model::CreateFromFile(modelPath_);
                        useModelName_ = modelPath_;
                        strcpy_s(modelName_, 256, "");
                    }
                    ImGui::Separator();
                    ImGui::Text("ModelName");
                    ImGui::InputText("##ModelName", modelName_, 256);
                    if (ImGui::Button("Apply##ModelName"))
                    {
                        Model* model = ModelManager::GetInstance()->FindSameModel(modelName_);
                        if (model == nullptr)
                        {
                            throw std::runtime_error("Model not found");
                        }
                        else
                        {
                            useModelName_ = modelName_;
                            strcpy_s(modelPath_, 256, "");
                        }
                    }
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Use Texture"))
                {
                    ImGui::Text("TextureRoot");
                    ImGui::InputText("##TextureRoot", textureRoot_, 256);

                    ImGui::Text("TexturePath");
                    ImGui::InputText("##TexturePath", texturePath_, 256);
                    if (ImGui::Button("Apply##TexturePath"))
                    {
                        initParams_.textureHandle = TextureManager::GetInstance()->Load(texturePath_, textureRoot_);
                    }
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Billboard"))
                {
                    ImGui::Text("Billboard");
                    ImGui::Checkbox("X", &initParams_.billboard[0]);            ImGui::SameLine();
                    ImGui::Checkbox("Y", &initParams_.billboard[1]);            ImGui::SameLine();
                    ImGui::Checkbox("Z", &initParams_.billboard[2]);
                    ImGui::TreePop();
                }

                // RenderSetting
                if (ImGui::TreeNode("Render Setting"))
                {
                    ImGui::Text("Blend Mode");
                    ImGui::RadioButton("Normal", reinterpret_cast<int*>(&blendMode_), static_cast<int>(BlendMode::Normal));
                    ImGui::RadioButton("Add", reinterpret_cast<int*>(&blendMode_), static_cast<int>(BlendMode::Add));
                    ImGui::RadioButton("Sub", reinterpret_cast<int*>(&blendMode_), static_cast<int>(BlendMode::Sub));
                    ImGui::RadioButton("Mul", reinterpret_cast<int*>(&blendMode_), static_cast<int>(BlendMode::Multiply));
                    ImGui::RadioButton("Screen", reinterpret_cast<int*>(&blendMode_), static_cast<int>(BlendMode::Screen));

                    ImGui::Separator();

                    ImGui::Checkbox("Cull Back", &cullBack_);

                    ImGui::TreePop();
                }

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
               /* if (ImGui::TreeNode("Sequence"))
                {
                    ImGui::Text("Sequence For Particle");
                    ImGui::Checkbox("Enable Sequence", &initParams_.enableSequence);
                    if(initParams_.enableSequence)
                        ImGui::Checkbox("Show Sequence", &isOpenTimeline);
                    ImGui::TreePop();
                }*/
            }

        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
    ImGui::Separator();

#endif // _DEBUG

}

void ParticleEmitter::GenerateParticles()
{
    std::vector<Particle*> particles(emitCount_);

    for (uint32_t i = 0; i < emitCount_; ++i)
    {

        ParticleInitParam initParam;

        //particle.acceleration;

        initParam.isBillboard;
        initParam.isBillboard = initParams_.billboard;

        initParam.color;
        Vector3 rgb = initParams_.colorRGB.GetValue();
        float alpha = initParams_.colorA.GetValue();

        initParam.color = Vector4(rgb.x, rgb.y, rgb.z, alpha);


        initParam.direction;
        if (initParams_.directionType == ParticleDirectionType::Random ||
            initParams_.directionType == ParticleDirectionType::Fixed)
        {
            initParam.direction = initParams_.direction.GetValue();
        }
        else if (initParams_.directionType == ParticleDirectionType::Outward)
        {
            //TODO
            initParam.direction = Vector3(0, 1, 0);
        }
        else if (initParams_.directionType == ParticleDirectionType::Inward)
        {
            initParam.direction = Vector3(0, -1, 0);
        }


        initParam.isInfiniteLife;
        initParam.isInfiniteLife = initParams_.lifeTimeType == ParticleLifeTimeType::Infinite;

        initParam.lifeTime;
        if (!initParam.isInfiniteLife)
        {
            initParam.lifeTime = initParams_.lifeTime.GetValue();
        }

        initParam.position;
        Vector3 emitterWorldPos = offset_;
        if (parentTransform_)
            parentTransform_->GetWorldPosition();
        switch (shape_)
        {
        case EmitterShape::Box:
        {
            Vector3 boxOffset = RandomGenerator::GetInstance()->GetRandValue(initParams_.boxOffset.GetValue(), boxSize_ / 2.0f);
            initParam.position = emitterWorldPos + boxOffset;
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

        initParam.rotate;


        initParam.size;
        initParam.size = initParams_.size.GetValue();


        initParam.speed;
        initParam.speed = initParams_.speed.GetValue();

        particles[i] = new Particle();
        particles[i]->Initialize(initParam);
    }

    ParticleRenderSettings settings;
    settings.blendMode = blendMode_;
    settings.cullBack = cullBack_;

    if (useModelName_ == "")
        useModelName_ = "plane/plane.gltf";

    // groupnameには仮でエミッターの名前を入れている
    ParticleSystem::GetInstance()->AddParticles(name_, useModelName_, particles, settings, initParams_.textureHandle, initParams_.modifiers);
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

    jsonBinder_->RegisterVariable("BoxOffset_Random", &initParams_.boxOffset.isRandom);
    jsonBinder_->RegisterVariable("BoxOffset_Min", &initParams_.boxOffset.min);
    jsonBinder_->RegisterVariable("BoxOffset_Max", &initParams_.boxOffset.max);
    jsonBinder_->RegisterVariable("BoxOffset_Max", &initParams_.boxOffset.value);

    jsonBinder_->RegisterVariable("SphereOffset_Random", &initParams_.sphereOffset.isRandom);
    jsonBinder_->RegisterVariable("SphereOffset_Min", &initParams_.sphereOffset.min);
    jsonBinder_->RegisterVariable("SphereOffset_Max", &initParams_.sphereOffset.max);
    jsonBinder_->RegisterVariable("SphereOffset_Value", &initParams_.sphereOffset.value);

    jsonBinder_->RegisterVariable("direction_Type", reinterpret_cast<uint32_t*>(&initParams_.directionType));
    jsonBinder_->RegisterVariable("direction_Min", &initParams_.direction.min);
    jsonBinder_->RegisterVariable("direction_Max", &initParams_.direction.max);
    jsonBinder_->RegisterVariable("direction_Value", &initParams_.direction.value);
    jsonBinder_->RegisterVariable("direction_Random", &initParams_.direction.isRandom);

    jsonBinder_->RegisterVariable("speed_Random", &initParams_.speed.isRandom);
    jsonBinder_->RegisterVariable("speed_Min", &initParams_.speed.min);
    jsonBinder_->RegisterVariable("speed_Max", &initParams_.speed.max);
    jsonBinder_->RegisterVariable("speed_Value", &initParams_.speed.value);

    jsonBinder_->RegisterVariable("deceleration_Random", &initParams_.deceleration.isRandom);
    jsonBinder_->RegisterVariable("deceleration_Min", &initParams_.deceleration.min);
    jsonBinder_->RegisterVariable("deceleration_Max", &initParams_.deceleration.max);
    jsonBinder_->RegisterVariable("deceleration_Value", &initParams_.deceleration.value);

    jsonBinder_->RegisterVariable("lifeTime_Type", reinterpret_cast<uint32_t*>(&initParams_.lifeTimeType));
    jsonBinder_->RegisterVariable("lifeTime_Min", &initParams_.lifeTime.min);
    jsonBinder_->RegisterVariable("lifeTime_Max", &initParams_.lifeTime.max);
    jsonBinder_->RegisterVariable("lifeTime_Value", &initParams_.lifeTime.value);
    jsonBinder_->RegisterVariable("lifeTime_Random", &initParams_.lifeTime.isRandom);

    jsonBinder_->RegisterVariable("size_Random", &initParams_.size.isRandom);
    jsonBinder_->RegisterVariable("size_Min", &initParams_.size.min);
    jsonBinder_->RegisterVariable("size_Max", &initParams_.size.max);
    jsonBinder_->RegisterVariable("size_Value", &initParams_.size.value);

    jsonBinder_->RegisterVariable("RGB_Random", &initParams_.colorRGB.isRandom);
    jsonBinder_->RegisterVariable("RGB_Min", &initParams_.colorRGB.min);
    jsonBinder_->RegisterVariable("RGB_Max", &initParams_.colorRGB.max);
    jsonBinder_->RegisterVariable("RGB_Value", &initParams_.colorRGB.value);

    jsonBinder_->RegisterVariable("alpha_Random", &initParams_.colorA.isRandom);
    jsonBinder_->RegisterVariable("alpha_Min", &initParams_.colorA.min);
    jsonBinder_->RegisterVariable("alpha_Max", &initParams_.colorA.max);
    jsonBinder_->RegisterVariable("alpha_Value", &initParams_.colorA.value);

    jsonBinder_->RegisterVariable("textureHandle", &initParams_.textureHandle);

    jsonBinder_->RegisterVariable("modifiers", &initParams_.modifiers);

    jsonBinder_->RegisterVariable("cullBack", &cullBack_);
    jsonBinder_->RegisterVariable("blendMode", reinterpret_cast<uint32_t*>(&blendMode_));


}

#pragma region DebugWindow

#ifdef _DEBUG

void ParticleEmitter::DebugWindowForSize()
{
    auto& sizeParams = initParams_.size;

    int imSize = static_cast<int>(!sizeParams.isRandom);

    ImGui::RadioButton("Random", &imSize, 0);
    ImGui::RadioButton("Fixed", &imSize, 1);

    sizeParams.isRandom = imSize == 0;

    if (sizeParams.isRandom)
    {
        ImGui::DragFloat("Min", &sizeParams.min, 0.01f);
        ImGui::DragFloat("Max", &sizeParams.max, 0.01f);
    }
    else
    {
        ImGui::DragFloat("Size", &sizeParams.value);
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForPosition()
{
    switch (shape_)
    {
    case EmitterShape::Box:
    {
        auto& boxOffset = initParams_.boxOffset;

        int imBoxOffset = static_cast<int>(!boxOffset.isRandom);

        ImGui::RadioButton("Random", &imBoxOffset, 0);
        ImGui::RadioButton("Fixed", &imBoxOffset, 1);

        boxOffset.isRandom = imBoxOffset == 0;

        if (boxOffset.isRandom)
        {
            ImGui::DragFloat3("Min", &boxOffset.min.x, 0.01f);
            ImGui::DragFloat3("Max", &boxOffset.max.x, 0.01f);
        }
        else
        {
            ImGui::DragFloat3("Box Offset", &boxOffset.value.x, 0.01f);
        }
    }
    break;
    case EmitterShape::Sphere:
    {
        auto& sphereOffset = initParams_.sphereOffset;
        int imSphereOffset = static_cast<int>(!sphereOffset.isRandom);

        ImGui::RadioButton("Random", &imSphereOffset, 0);
        ImGui::RadioButton("Fixed", &imSphereOffset, 1);

        sphereOffset.isRandom = imSphereOffset == 0;

        if (sphereOffset.isRandom)
        {
            ImGui::DragFloat("Min", &sphereOffset.min, 0.01f);
            ImGui::DragFloat("Max", &sphereOffset.max, 0.01f);
        }
        else
        {
            ImGui::DragFloat("Sphere Offset", &sphereOffset.value, 0.01f);
        }
    }
    break;
    default:
        break;
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForDirection()
{
    int imDirection = static_cast<int>(initParams_.directionType);

    ImGui::RadioButton("Outward", &imDirection, static_cast<int>(ParticleDirectionType::Outward));
    ImGui::RadioButton("Inward", &imDirection, static_cast<int>(ParticleDirectionType::Inward));
    ImGui::RadioButton("Random", &imDirection, static_cast<int>(ParticleDirectionType::Random));
    ImGui::RadioButton("Fixed", &imDirection, static_cast<int>(ParticleDirectionType::Fixed));

    initParams_.directionType = static_cast<ParticleDirectionType>(imDirection);

    if (initParams_.directionType == ParticleDirectionType::Random)
    {
        initParams_.direction.isRandom = true;
        ImGui::DragFloat3("Min", &initParams_.direction.min.x, 0.01f);
        ImGui::DragFloat3("Max", &initParams_.direction.max.x, 0.01f);
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

    speed.isRandom = imSpeed == 0;

    if (speed.isRandom)
    {
        ImGui::DragFloat("Min", &speed.min, 0.01f);
        ImGui::DragFloat("Max", &speed.max, 0.01f);
    }
    else
    {
        ImGui::DragFloat("Speed", &speed.value, 0.01f);
    }
    ImGui::Separator();

}

void ParticleEmitter::DebugWindowForDeceleration()
{
    auto& deceleration = initParams_.deceleration;
    int imDece = static_cast<int>(!deceleration.isRandom);
    ImGui::RadioButton("Random", &imDece, 0);
    ImGui::RadioButton("Fixed", &imDece, 1);

    deceleration.isRandom = imDece == 0;

    if (deceleration.isRandom)
    {
        ImGui::DragFloat("Min", &deceleration.min, 0.01f);
        ImGui::DragFloat("Max", &deceleration.max, 0.01f);
    }
    else
    {
        ImGui::DragFloat("Deceleration", &deceleration.value, 0.01f);
    }
    ImGui::Separator();
}

void ParticleEmitter::DebugWindowForLifeTime()
{
    int lifeTimePtr = static_cast<int>(initParams_.lifeTimeType);
    ImGui::RadioButton("Infinite", &lifeTimePtr, static_cast<int>(ParticleLifeTimeType::Infinite));
    ImGui::RadioButton("Random", &lifeTimePtr, static_cast<int>(ParticleLifeTimeType::Random));
    ImGui::RadioButton("Fixed", &lifeTimePtr, static_cast<int>(ParticleLifeTimeType::Fixed));

    initParams_.lifeTimeType = static_cast<ParticleLifeTimeType>(lifeTimePtr);

    if (initParams_.lifeTimeType == ParticleLifeTimeType::Random)
    {
        initParams_.lifeTime.isRandom = true;

        ImGui::DragFloat("Min", &initParams_.lifeTime.min, 0.01f);
        ImGui::DragFloat("Max", &initParams_.lifeTime.max, 0.01f);
    }
    else if (initParams_.lifeTimeType == ParticleLifeTimeType::Fixed)
    {
        initParams_.lifeTime.isRandom = false;
        ImGui::DragFloat("Life Time", &initParams_.lifeTime.value, 0.01f);
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

        color.isRandom = colorPtr == 0;

        if (color.isRandom)
        {
            ImGui::ColorEdit3("Min", &color.min.x);
            ImGui::ColorEdit3("Max", &color.max.x);
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

        alpha.isRandom = imAlpha == 0;

        if (alpha.isRandom)
        {
            ImGui::DragFloat("Min", &alpha.min, 0.01f);
            ImGui::DragFloat("Max", &alpha.max, 0.01f);
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
    ImGui::InputText("##ModifierName", modifierName, 256);
    if (ImGui::Button("Add Modifier"))
    {
        initParams_.modifiers.push_back(modifierName);
        strcpy_s(modifierName, 256, "");
    }
    static int removeIndex = -1;
    ImGui::InputInt("Remove Index", &removeIndex);
    if (ImGui::Button("Remove Modifier"))
    {
        if (removeIndex >= 0 && removeIndex < static_cast<int>(initParams_.modifiers.size()))
        {
            initParams_.modifiers.erase(initParams_.modifiers.begin() + removeIndex);
            removeIndex = -1;
        }
    }

    ImGui::Separator();

    if (ImGui::TreeNode("Modifier List"))
    {

        for (auto& modifier : initParams_.modifiers)
        {
            ImGui::Text(modifier.c_str());
        }
        ImGui::TreePop();
    }

    ImGui::Separator();
}
#endif // _DEBUG
#pragma endregion