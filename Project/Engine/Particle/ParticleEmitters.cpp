#include "ParticleEmitters.h"
#include "ParticleManager.h"
#include "ParticleInitParam.h"

#include "LineDrawer/LineDrawer.h"
#include "Math/MatrixFunction.h"
#include "Math/VectorFunction.h"
#include "Utility/ConfigManager.h"
#include "Utility/RandomGenerator.h"
#include "ImGuiManager/ImGuiManager.h"
#include "TextureManager/TextureManager.h"

void ParticleEmitter::Setting(const std::string& _name)
{
    name_ = _name;

    ConfigManager* instance = ConfigManager::GetInstance();

    instance->SetVariable(name_, "lifeTime_min", &setting_.lifeTime.min);
    instance->SetVariable(name_, "lifeTime_max", &setting_.lifeTime.max);
    instance->SetVariable(name_, "size_min", &setting_.size.min);
    instance->SetVariable(name_, "size_max", &setting_.size.max);
    instance->SetVariable(name_, "rotate_min", &setting_.rotate.min);
    instance->SetVariable(name_, "rotate_max", &setting_.rotate.max);
    instance->SetVariable(name_, "spped_min", &setting_.spped.min);
    instance->SetVariable(name_, "spped_max", &setting_.spped.max);
    instance->SetVariable(name_, "direction_min", &setting_.direction.min);
    instance->SetVariable(name_, "direction_max", &setting_.direction.max);
    instance->SetVariable(name_, "acceleration_min", &setting_.acceleration.min);
    instance->SetVariable(name_, "acceleration_max", &setting_.acceleration.max);
    instance->SetVariable(name_, "color_min", &setting_.color.min);
    instance->SetVariable(name_, "color_max", &setting_.color.max);

    instance->SetVariable(name_, "countPerEmit", &countPerEmit_);
    instance->SetVariable(name_, "emitPerSec", &emitPerSec_);
    instance->SetVariable(name_, "maxParticles", &maxParticles_);
    instance->SetVariable(name_, "emitRepeatCount", &emitRepeatCount_);

    //instance->SetVariable(name_, "randomColor", reinterpret_cast<uint32_t*> (&randomColor_));
    instance->SetVariable(name_, "fadeAlpha", reinterpret_cast<uint32_t*> (&fadeAlpha_));
    instance->SetVariable(name_, "fadeStartRatio", &fadeStartRatio_);
    instance->SetVariable(name_, "delayTime", &delayTime_);
    instance->SetVariable(name_, "loop", reinterpret_cast<uint32_t*>(&loop_));
    instance->SetVariable(name_, "changeColor", reinterpret_cast<uint32_t*>(&changeColor_));
    instance->SetVariable(name_, "changeSize", reinterpret_cast<uint32_t*>(&changeSize_));
    instance->SetVariable(name_, "useBillboard", reinterpret_cast<uint32_t*>(&isEnableBillboard_));

    instance->SetVariable(name_, "shape", reinterpret_cast<uint32_t*>(&shape_));
    instance->SetVariable(name_, "direction", reinterpret_cast<uint32_t*>(&particleDirection_));
    instance->SetVariable(name_, "size", &size_);
    instance->SetVariable(name_, "radius", &radius_);
    instance->SetVariable(name_, "offset", &offset_);
    instance->SetVariable(name_, "rotate", &rotate_);
    instance->SetVariable(name_, "position", &position_);

    instance->SetVariable(name_, "modelPath", &useModelPath_);
    instance->SetVariable(name_, "texturePath", &useTextruePath_);


    emitTime_ = 1.0f / static_cast<float> (emitPerSec_);

    switch (shape_)
    {
    case EmitterShape::Box:
        SetShape_Box(size_);
        break;
    case EmitterShape::Shpere:
        SetShape_Sphere(radius_);
        break;
    case EmitterShape::Circle:
        SetShape_Circle(radius_);
        break;
    case EmitterShape::None:
        break;
    default:
        break;
    }

    if (useModelPath_.empty())
        useModelPath_ = "plane/plane.gltf";

    if (useTextruePath_.empty())
        useTextruePath_ = "circle.png";

    uint32_t handle = TextureManager::GetInstance()->Load(useTextruePath_);
    ParticleManager::GetInstance()->CreateParticleGroup(name_, useModelPath_, this, handle);

}

void ParticleEmitter::Update()
{
    if (!isActive_ ||
        !isAlive_) {
        return;
    }

    if (parentMatWorld_)
        position_ = Transform(offset_, *parentMatWorld_);
    else
        position_ = position_ + offset_;

    currentTime_ += deltaTime_;

    // エミッターの持続時間が経過していたらスキップ

    if (emitTime_ <= currentTime_)
    {
        // loop しない場合 かつ emit回数が emit回数に達した場合
        if (!loop_ && emitRepeatCount_ <= emitCount_)
        {
            isActive_ = false;
            isAlive_ = false;
            emitCount_ = 0;
        }

        std::vector<Particle> particles;

        for (uint32_t count = 0; count < countPerEmit_; ++count)
        {
            particles.push_back(GenerateParticleData());
        }

        ParticleManager::GetInstance()->AddParticleToGroup(name_, particles);
        currentTime_ = 0;
        if(!loop_)emitCount_++;
    }

}

void ParticleEmitter::Draw()
{
#ifndef _DEBUG
    return;
#endif // _DEBUG

    switch (shape_)
    {
    case EmitterShape::Box:
        {
            Matrix4x4 affine = MakeAffineMatrix(size_, rotate_, position_);
            LineDrawer::GetInstance()->DrawOBB(affine);
        }
        break;
    case EmitterShape::Shpere:
        Matrix4x4 affine = MakeAffineMatrix({ radius_ }, rotate_, position_ );
        LineDrawer::GetInstance()->DrawSphere(affine);
        break;
    case EmitterShape::Circle:
        break;
    case EmitterShape::None:
        break;
    default:
        break;
    }
}

void ParticleEmitter::SetShape_Box(const Vector3& _size)
{
    shape_ = EmitterShape::Box;
    size_ = _size;
}

void ParticleEmitter::SetShape_Sphere(float _radius)
{
    shape_ = EmitterShape::Shpere;
    radius_ = _radius;
}

void ParticleEmitter::SetShape_Circle(float _radius)
{
    shape_ = EmitterShape::Circle;
    radius_ = _radius;
}

void ParticleEmitter::SetActive(bool _active)
{
    currentTime_ = emitTime_;
    isActive_ = _active;
}

void ParticleEmitter::ShowDebugWinsow()
{
#ifdef _DEBUG

    static const char* shapeCombo[1024] = { "Box","Sphere","Circle","None" };
    static const char* directionCombo[1024] = { "inward","outward","random" };

    ImGui::BeginTabBar("Emitter");
    if (ImGui::BeginTabItem(name_.c_str()))
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));

        if (ImGui::TreeNodeEx("Emitter", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::Combo("shape", reinterpret_cast<int*>(&shape_), shapeCombo, 4);
            ImGui::Combo("direction", reinterpret_cast<int*>(&particleDirection_), directionCombo, 3);


            ImGui::SeparatorText("Emitter");
            if (shape_ == EmitterShape::Box)
                ImGui::DragFloat3("size", &size_.x, 0.01f);
            else if (shape_ == EmitterShape::Shpere || shape_ == EmitterShape::Circle)
                ImGui::DragFloat("radius", &radius_, 0.01f);

            ImGui::DragFloat3("position", &position_.x, 0.01f);
            ImGui::DragFloat3("offset", &offset_.x, 0.01f);
            ImGui::DragInt("countPerEmit", reinterpret_cast<int*>(&countPerEmit_), 1, 0);
            if (ImGui::DragInt("emitPerSec", reinterpret_cast<int*>(&emitPerSec_), 1, 0))
                emitTime_ = 1.0f / static_cast<float>(emitPerSec_);
            ImGui::InputInt("maxParticles", reinterpret_cast<int*>(&maxParticles_), 1);
            ImGui::InputInt("emitRepeatCount", reinterpret_cast<int*>(&emitRepeatCount_), 1);
            ImGui::DragFloat("delayTime", &delayTime_, 0.01f);
            ImGui::DragFloat("duration", &duration_, 0.01f);

            ImGui::DragFloat("fadeStartRatio", &fadeStartRatio_, 0.01f, 0, 1);

            ImGui::Columns(2, "mycolumns", false);
            ImGui::Checkbox("randomColor", &randomColor_);
            ImGui::Checkbox("fadeAlpha", &fadeAlpha_);

            ImGui::Checkbox("loop", &loop_);
            ImGui::NextColumn();
            ImGui::Checkbox("changeColor", &changeColor_);

            ImGui::Checkbox("changeSize", &changeSize_);
            ImGui::Checkbox("useBillboard", &isEnableBillboard_);

            ImGui::Columns(1);

            ImGui::SeparatorText("use path");
            ImGui::InputText("Model", name_buffer_, 256);
            useModelPath_ = name_buffer_;
            if (ImGui::Button("Set"))
                ParticleManager::GetInstance()->SetGroupModel(name_, useModelPath_);

            ImGui::InputText("Texture", texture_buffer_, 256);
            useTextruePath_ = texture_buffer_;
            if (ImGui::Button("Set"))
                ParticleManager::GetInstance()->SetGroupTexture(name_, TextureManager::GetInstance()->Load(useTextruePath_));

            ImGui::TreePop();

        }

        if (ImGui::TreeNodeEx("Particle_Init", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloatRange2("lifeTime", &setting_.lifeTime.min, &setting_.lifeTime.max, 0.01f);
            ImGui::DragFloat3("size_min", &setting_.size.min.x, 0.01f);
            ImGui::DragFloat3("size_max", &setting_.size.max.x, 0.01f);
            ImGui::DragFloat3("rotate_min", &setting_.rotate.min.x, 0.01f);
            ImGui::DragFloat3("rotate_max", &setting_.rotate.max.x, 0.01f);
            ImGui::DragFloatRange2("spped", &setting_.spped.min, &setting_.spped.max, 0.01f);
            ImGui::DragFloat3("direction_min", &setting_.direction.min.x, 0.01f);
            ImGui::DragFloat3("direction_max", &setting_.direction.max.x, 0.01f);
            ImGui::DragFloat3("acceleration_min", &setting_.acceleration.min.x, 0.01f);
            ImGui::DragFloat3("acceleration_max", &setting_.acceleration.max.x, 0.01f);
            ImGui::ColorEdit4("color_min", &setting_.color.min.x);
            ImGui::ColorEdit4("color_max", &setting_.color.max.x);
            ImGui::TreePop();
        }
        ImGui::PopStyleColor();

        if (ImGui::Button("save"))
        {
            ConfigManager::GetInstance()->SaveData(name_);
        }

        if (ImGui::Button("add"))
        {
            std::vector<Particle> particles;

            for (uint32_t count = 0; count < countPerEmit_; ++count)
            {
                particles.push_back(GenerateParticleData());
            }

            ParticleManager::GetInstance()->AddParticleToGroup(name_, particles);
            currentTime_ = 0;
        }
        ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
#endif // _DEBUG
}

void ParticleEmitter::Reset()
{
    currentTime_ = 0;
    emitCount_ = 0;
    isActive_ = false;
    isAlive_ = true;
}


Particle ParticleEmitter::GenerateParticleData()
{
    auto random = RandomGenerator::GetInstance();
    ParticleInitParam param;

    param.lifeTime = random->GetUniformFloat(setting_.lifeTime.min, setting_.lifeTime.max);
    param.rotate = random->GetUniformVec3(setting_.rotate.min, setting_.rotate.max);
    param.speed = random->GetUniformFloat(setting_.spped.min, setting_.spped.max);
    param.acceleration = random->GetUniformVec3(setting_.acceleration.min, setting_.acceleration.max);

    if (changeSize_)
    {
        param.currentSize = setting_.size.min;
        param.startSize = setting_.size.min;
        param.endSize = setting_.size.max;
    }
    else
        param.currentSize = random->GetUniformVec3(setting_.size.min, setting_.size.max);

    if(randomColor_)
        param.currentColor = random->GetUniformColor();
    else
    {
        if (changeColor_)
        {
            param.currentColor = setting_.color.min;
            param.startColor = setting_.color.min;
            param.endColor = setting_.color.max;
        }
        else
        {
            param.currentColor = random->GetUniformVec4(setting_.color.min, setting_.color.max);
        }
    }


    switch (shape_)
    {
    case EmitterShape::Box:
        {
            Vector3 halfSize = size_ / 2.0f;
            param.position = random->GetUniformVec3(-halfSize, halfSize);
        }
        break;
    case EmitterShape::Shpere:
        {
            float rad = random->GetUniformAngle();
            param.position.x = std::cosf(rad)*radius_;
            param.position.y = random->GetUniformFloat(-radius_, radius_);
            param.position.z = std::sinf(rad) * radius_;
        }
        break;
    case EmitterShape::Circle:
        {
            float rad = random->GetUniformAngle();
            param.position.x = std::cosf(rad) * radius_;
            param.position.y = 0;
            param.position.z = std::sinf(rad) * radius_;
        }
        break;
    case EmitterShape::None:
    default:
        throw std::runtime_error("Unknown Emitter Shape");
        break;
    }

    switch (particleDirection_)
    {
    case ParticleDirection::Inward:
        param.direction = -param.position.Normalize();
        break;
    case ParticleDirection::Outward:
        param.direction = param.position.Normalize();
        break;
    case ParticleDirection::Random:
        param.direction = random->GetUniformVec3(setting_.direction.min, setting_.direction.max);
        break;
    default:
        break;
    }

        param.position += position_;

    param.changeColor = changeColor_;
    param.changeSize = changeSize_;
    param.isFade = fadeAlpha_;
    param.fadeRatio = fadeStartRatio_;

    Particle particle;
    particle.Initialize(param);

    return particle;
}