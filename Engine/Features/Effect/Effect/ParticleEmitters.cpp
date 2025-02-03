#include <Features/Effect/Effect/ParticleEmitters.h>
#include <Features/Effect/Manager/ParticleManager.h>
#include <Features/Effect/ParticleInitParam.h>

#include <Math/Easing.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Math/Vector/VectorFunction.h>
#include <Math/MyLib.h>
#include <Math/Random/RandomGenerator.h>
#include <Debug/ImGuiManager.h>
#include <Core/DXCommon/TextureManager/TextureManager.h>

void ParticleEmitter::Initialize(const std::string& _name)
{
    name_ = _name;

    jsonBinder_ = std::make_unique<JsonBinder>(_name, "Resources/Data/Particles/Emitters/");


    RegisterEmitParticleSettings();
    RegisterParticleInitParam();
    RegisterEmitterSettings();



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
        useTextruePath_ = "particle/circle.png";

    //parametor_.alphaTransition.keys.emplace_back(TransitionKeyFrame{ 1.0f ,1.0f });
    //parametor_.colorTransition.keys.emplace_back(TransitionKeyFrame{ 1.0f,Vector3{1,1,1} });
    //parametor_.rotateTransition.keys.emplace_back(TransitionKeyFrame{ 1.0f,Vector3{0,0,0} });
    //parametor_.sizeTransition.keys.emplace_back(TransitionKeyFrame{ 1.0f,Vector3{1,1,1} });
    //parametor_.speedTransition.keys.emplace_back(TransitionKeyFrame{ 1.0f,0.0f });


    //parametor_.alphaTransition.keys.emplace_front(TransitionKeyFrame{ 0.0f, 1.0f });
    //parametor_.colorTransition.keys.emplace_front(TransitionKeyFrame{ 0.0f, Vector3{1,1,1} });
    //parametor_.rotateTransition.keys.emplace_front(TransitionKeyFrame{ 0.0f, Vector3{0,0,0} });
    //parametor_.sizeTransition.keys.emplace_front(TransitionKeyFrame{ 0.0f, Vector3{1,1,1} });
    //parametor_.speedTransition.keys.emplace_front(TransitionKeyFrame{ 0.0f, 1.0f });

    gameTime_ = GameTime::GetInstance();

    uint32_t handle = TextureManager::GetInstance()->Load(useTextruePath_);
    ParticleManager::GetInstance()->CreateParticleGroup(name_, useModelPath_, this, BlendMode::Add, handle);

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

    deltaTime_ = gameTime_->GetChannel(timeChannel_).GetDeltaTime<float>();
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

void ParticleEmitter::Draw() const
{
#ifndef _DEBUG
    return;
#endif // _DEBUG

    if (!isDraw_)
        return;

    switch (shape_)
    {
    case EmitterShape::Box:
        {
            Matrix4x4 affine = MakeAffineMatrix(size_, rotate_, position_);
            LineDrawer::GetInstance()->DrawOBB(affine);
        }
        break;
    case EmitterShape::Shpere:
        {
            Matrix4x4 affine = MakeAffineMatrix({ radius_ }, rotate_, position_);
            LineDrawer::GetInstance()->DrawSphere(affine);
        }
        break;
    case EmitterShape::Circle:
        // TODO : 円の描画
        {
        }
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

std::array<bool, 3> ParticleEmitter::GetBillboardAxes() const
{
    std::array<bool, 3> axes;

    axes[0] = billboardAxes_.x == 0 ? false : true;
    axes[1] = billboardAxes_.y == 0 ? false : true;
    axes[2] = billboardAxes_.z == 0 ? false : true;

    return axes;
}


void ParticleEmitter::Reset()
{
    currentTime_ = 0;
    emitCount_ = 0;
    isActive_ = false;
    isAlive_ = true;
}

void ParticleEmitter::RegisterEmitParticleSettings()
{
    // 配列にしてもいいかも vector
    jsonBinder_->RegisterVariable("lifeTime_min", &setting_.lifeTime.value.min);
    jsonBinder_->RegisterVariable("lifeTime_max", &setting_.lifeTime.value.max);
    jsonBinder_->RegisterVariable("lifeTime_fixed", reinterpret_cast<uint32_t*>(&setting_.lifeTime.fixed));
    jsonBinder_->RegisterVariable("lifeTime_random", reinterpret_cast<uint32_t*>(&setting_.lifeTime.random));

    jsonBinder_->RegisterVariable("size_min", &setting_.size.value.min);
    jsonBinder_->RegisterVariable("size_max", &setting_.size.value.max);
    jsonBinder_->RegisterVariable("size_fixed", reinterpret_cast<uint32_t*>(&setting_.size.fixed));
    jsonBinder_->RegisterVariable("size_random", reinterpret_cast<uint32_t*>(&setting_.size.random));

    jsonBinder_->RegisterVariable("rotate_min", &setting_.rotate.value.min);
    jsonBinder_->RegisterVariable("rotate_max", &setting_.rotate.value.max);
    jsonBinder_->RegisterVariable("rotate_fixed", reinterpret_cast<uint32_t*>(&setting_.rotate.fixed));
    jsonBinder_->RegisterVariable("rotate_random", reinterpret_cast<uint32_t*>(&setting_.rotate.random));

    jsonBinder_->RegisterVariable("spped_min", &setting_.speed.value.min);
    jsonBinder_->RegisterVariable("spped_max", &setting_.speed.value.max);
    jsonBinder_->RegisterVariable("spped_fixed", reinterpret_cast<uint32_t*>(&setting_.speed.fixed));
    jsonBinder_->RegisterVariable("spped_random", reinterpret_cast<uint32_t*>(&setting_.speed.random));

    jsonBinder_->RegisterVariable("direction_min", &setting_.direction.value.min);
    jsonBinder_->RegisterVariable("direction_max", &setting_.direction.value.max);
    jsonBinder_->RegisterVariable("direction_fixed", reinterpret_cast<uint32_t*>(&setting_.direction.fixed));
    jsonBinder_->RegisterVariable("direction_random", reinterpret_cast<uint32_t*>(&setting_.direction.random));

    jsonBinder_->RegisterVariable("acceleration_min", &setting_.acceleration.value.min);
    jsonBinder_->RegisterVariable("acceleration_max", &setting_.acceleration.value.max);
    jsonBinder_->RegisterVariable("acceleration_fixed", reinterpret_cast<uint32_t*>(&setting_.acceleration.fixed));
    jsonBinder_->RegisterVariable("acceleration_random", reinterpret_cast<uint32_t*>(&setting_.acceleration.random));

    jsonBinder_->RegisterVariable("color_min", &setting_.color.value.min);
    jsonBinder_->RegisterVariable("color_max", &setting_.color.value.max);
    jsonBinder_->RegisterVariable("color_fixed", reinterpret_cast<uint32_t*>(&setting_.color.fixed));
    jsonBinder_->RegisterVariable("color_random", reinterpret_cast<uint32_t*>(&setting_.color.random));

}

void ParticleEmitter::RegisterParticleInitParam()
{
    jsonBinder_->RegisterVariable("lifeTime", &parametor_.lifeTime);
    jsonBinder_->RegisterVariable("isInfiniteLife", reinterpret_cast<uint32_t*>(&parametor_.isInfiniteLife));
    jsonBinder_->RegisterVariable("size", &parametor_.size);
    jsonBinder_->RegisterVariable("rotate", &parametor_.rotate);
    jsonBinder_->RegisterVariable("position", &parametor_.position);
    jsonBinder_->RegisterVariable("speed", &parametor_.speed);
    jsonBinder_->RegisterVariable("direction", &parametor_.direction);
    jsonBinder_->RegisterVariable("acceleration", &parametor_.acceleration);
    jsonBinder_->RegisterVariable("color", &parametor_.color);

    jsonBinder_->GetVariableValue("speedTransition", parametor_.speedTransition);
    jsonBinder_->GetVariableValue("sizeTransition", parametor_.sizeTransition);
    jsonBinder_->GetVariableValue("rotateTransition", parametor_.rotateTransition);
    jsonBinder_->GetVariableValue("colorTransition", parametor_.colorTransition);
    jsonBinder_->GetVariableValue("alphaTransition", parametor_.alphaTransition);

}

void ParticleEmitter::RegisterEmitterSettings()
{
    jsonBinder_->RegisterVariable("shape", reinterpret_cast<uint32_t*>(&shape_));
    jsonBinder_->RegisterVariable("emitDirection", reinterpret_cast<uint32_t*>(&particleDirection_));

    jsonBinder_->RegisterVariable("position", &position_);
    jsonBinder_->RegisterVariable("offset", &offset_);
    jsonBinder_->RegisterVariable("rotate", &rotate_);
    jsonBinder_->RegisterVariable("size", &size_);
    jsonBinder_->RegisterVariable("radius", &radius_);

    jsonBinder_->RegisterVariable("loop", reinterpret_cast<uint32_t*>(&loop_));
    jsonBinder_->RegisterVariable("useBillboard", reinterpret_cast<uint32_t*>(&isEnableBillboard_));
    jsonBinder_->RegisterVariable("EnableLengthScaling", reinterpret_cast<uint32_t*>(&isLengthScalingEnabled_));
    jsonBinder_->RegisterVariable("lengthScaling", &lengthScaling_);


    jsonBinder_->RegisterVariable("billBoardAxes", &billboardAxes_);

    jsonBinder_->RegisterVariable("delayTime", &delayTime_);
    jsonBinder_->RegisterVariable("duration", &duration_);

    jsonBinder_->RegisterVariable("maxParticles", &maxParticles_);
    jsonBinder_->RegisterVariable("countPerEmit", &countPerEmit_);
    jsonBinder_->RegisterVariable("emitPerSec", &emitPerSec_);
    jsonBinder_->RegisterVariable("emitRepeatCount", &emitRepeatCount_);


    jsonBinder_->RegisterVariable("modelPath", &useModelPath_);
    jsonBinder_->RegisterVariable("texturePath", &useTextruePath_);
}


Particle ParticleEmitter::GenerateParticleData()
{
    auto random = RandomGenerator::GetInstance();

    ParticleInitParam param = parametor_;

    // lifeTime
    param.lifeTime = random->GetUniformFloat(setting_.lifeTime.value.min, setting_.lifeTime.value.max);

    // Position
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
            param.position.x = std::cosf(rad) * radius_;
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


    // acceleration
    param.acceleration = random->GetUniformVec3(setting_.acceleration.value.min, setting_.acceleration.value.max);


    // color
    if(setting_.color.fixed)
        param.color = setting_.color.value.min;
    else
        param.color = random->GetUniformVec4(setting_.color.value.min, setting_.color.value.max);


    // direction
    switch (particleDirection_)
    {
    case ParticleDirection::Inward:
        param.direction = -param.position.Normalize();
        break;
    case ParticleDirection::Outward:
        param.direction = param.position.Normalize();
        break;
    case ParticleDirection::Random:
        param.direction = random->GetUniformVec3(setting_.direction.value.min, setting_.direction.value.max);
        break;
    default:
        break;
    }

    // speed
    param.speed = random->GetUniformFloat(setting_.speed.value.min, setting_.speed.value.max);

    // rotate
    param.rotate = random->GetUniformVec3(setting_.rotate.value.min, setting_.rotate.value.max);

    // size
    param.size = random->GetUniformVec3(setting_.size.value.min, setting_.size.value.max);


    if (isLengthScalingEnabled_)
    {
        param.sizeTransition.isChange = false;
        param.size = setting_.size.value.min;
        param.size.x *= lengthScaling_;
        param.size.y /= lengthScaling_;
        //param.currentSize.x += 0.25f * param.speed;

        param.directionMatrix = DirectionToDirection({ 1,0,0 }, param.direction);
    }
    param.position += position_;// エミッタの座標を加算

    Particle particle;
    particle.Initialize(param);

    return particle;
}


std::string ParticleEmitter::GetTimeChannel()
{
    if (timeChannel_.empty())
        timeChannel_ = "default";
    return timeChannel_;
}

void ParticleEmitter::Save() const
{
    jsonBinder_->SetFolderPath();

    jsonBinder_->SendVariable("sizeTransition", parametor_.sizeTransition);
    jsonBinder_->SendVariable("rotateTransition", parametor_.rotateTransition);
    jsonBinder_->SendVariable("speedTransition", parametor_.speedTransition);
    jsonBinder_->SendVariable("colorTransition", parametor_.colorTransition);
    jsonBinder_->SendVariable("alphaTransition", parametor_.alphaTransition);

    jsonBinder_->Save();
}

bool ParticleEmitter::ShowDebugWinsow()
{
#ifdef _DEBUG

    static const char* shapeCombo[1024] = { "Box","Sphere","Circle","None" };
    static const char* directionCombo[1024] = { "inward","outward","random" };

    ImGui::BeginTabBar("Emitter");
    if (ImGui::BeginTabItem(name_.c_str()))
    {
        ImGui::Checkbox("Draw", &isDraw_);

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.8f, 0.2f, 0.2f, 0.5f));

        DisPlayEmitterParameters();

        if (ImGui::TreeNodeEx("Particle_Init", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.58f, 0.71f));

            DisplayLifeTimeParameters();
            DisplaySizeParameters();
            DisplayRotateParameters();
            DisplayDirectionParameters();
            DisplaySpeedParameters();
            DisplayAccelerationParameters();
            DisplayColorParameters();

            ImGui::PopStyleColor();
            ImGui::TreePop();

        }
        ImGui::PopStyleColor();

        if (ImGui::Button("save"))
        {
            Save();
        }

        if (ImGui::Button("Emit"))
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
        ImGui::EndTabBar();
        return true;
    }
    ImGui::EndTabBar();

    return false;
#endif // _DEBUG
}


void ParticleEmitter::DisPlayEmitterParameters()
{
#ifdef _DEBUG
    static const char* shapeCombo[1024] = { "Box","Sphere","Circle","None" };
    static const char* directionCombo[1024] = { "inward","outward","random" };

    float width = ImGui::GetContentRegionAvail().x / 5.0f; // 利用可能な幅を3等分

    if (ImGui::TreeNodeEx("Emitter", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.22f, 0.58f, 0.71f));
        if (ImGui::TreeNodeEx("Shapes", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::Combo("shape", reinterpret_cast<int*>(&shape_), shapeCombo, 4);
            ImGui::Combo("direction", reinterpret_cast<int*>(&particleDirection_), directionCombo, 3);
            ImGui::SeparatorText("");
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("transform", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::DragFloat3("position", &position_.x, 0.01f);
            ImGui::DragFloat3("offset", &offset_.x, 0.01f);
            ImGui::DragFloat3("rotate", &rotate_.x, 0.01f);
            if (shape_ == EmitterShape::Box)
                ImGui::DragFloat3("size", &size_.x, 0.01f);
            else if (shape_ == EmitterShape::Shpere || shape_ == EmitterShape::Circle)
                ImGui::DragFloat("radius", &radius_, 0.01f);

            ImGui::SeparatorText("");
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Emit Setting", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::PushItemWidth(width * 2);
            ImGui::InputInt("countPerEmit", reinterpret_cast<int*>(&countPerEmit_), 1);
            if (ImGui::InputInt("emitPerSec", reinterpret_cast<int*>(&emitPerSec_), 1))
            {
                if (emitPerSec_ > 0)
                    emitTime_ = 1.0f / static_cast<float>(emitPerSec_);
                else emitTime_ = 0;
            }
            ImGui::InputInt("maxParticles", reinterpret_cast<int*>(&maxParticles_), 1);
            ImGui::InputInt("emitRepeatCount", reinterpret_cast<int*>(&emitRepeatCount_), 1);
            ImGui::PopItemWidth();
            ImGui::DragFloat("delayTime", &delayTime_, 0.01f);
            ImGui::DragFloat("duration", &duration_, 0.01f);
            ImGui::SeparatorText("");

            if (countPerEmit_ < 0)    countPerEmit_ = 0;
            if (emitPerSec_ < 0)    emitPerSec_ = 0;
            ImGui::TreePop();

        }

        if (ImGui::TreeNodeEx("Use Path", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::InputText("Model", name_buffer_, 256);
            useModelPath_ = name_buffer_;
            if (ImGui::Button("Model Set"))
                ParticleManager::GetInstance()->SetGroupModel(name_, useModelPath_);

            ImGui::InputText("Texture", texture_buffer_, 256);
            useTextruePath_ = texture_buffer_;
            if (ImGui::Button("Texture Set"))
                ParticleManager::GetInstance()->SetGroupTexture(name_, TextureManager::GetInstance()->Load(useTextruePath_));
            ImGui::SeparatorText("");
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Billboard", ImGuiTreeNodeFlags_Framed))
        {
            if (ImGui::Checkbox("Enable", &isEnableBillboard_))
            {
                if (!isEnableBillboard_)
                {
                    billboardAxes_ = { 0,0,0 };
                }
                else
                {
                    billboardAxes_ = { 1,1,1 };
                }
            }
            ImGui::BeginDisabled(!isEnableBillboard_);
            bool axes[3] = { billboardAxes_.x != 0,billboardAxes_.y != 0,billboardAxes_.z != 0 };
            ImGui::Checkbox("X", &axes[0]);            ImGui::SameLine();
            ImGui::Checkbox("Y", &axes[1]);            ImGui::SameLine();
            ImGui::Checkbox("Z", &axes[2]);

            ImGui::EndDisabled();
            for (int i = 0; i < 3; ++i)
            {
                if (axes[i] != (billboardAxes_[i] != 0))
                {
                    billboardAxes_[i] = axes[i] ? 1.f : 0.f;
                }
            }
            ImGui::TreePop();

        }

        if (ImGui::TreeNodeEx("LengthScaling", ImGuiTreeNodeFlags_Framed))
        {
            ImGui::Checkbox("Enable", &isLengthScalingEnabled_);
            ImGui::BeginDisabled(!isLengthScalingEnabled_);
            ImGui::DragFloat("LengthScaling", &lengthScaling_, 0.001f);
            ImGui::EndDisabled();
            ImGui::TreePop();
        }

        ImGui::TreePop();
        ImGui::PopStyleColor();
    }
#endif // _DEBUG
}

void ParticleEmitter::DisplayLifeTimeParameters()
{
#ifdef _DEBUG


    float width = ImGui::GetContentRegionAvail().x / 5.0f; // 利用可能な幅を3等分

    ImGui::PushID("LifeTime");
    if (ImGui::TreeNodeEx("LifeTime", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Columns(2, "lifetimechec", false);
        if (ImGui::Checkbox("Fixed", &setting_.lifeTime.fixed))
            setting_.lifeTime.random = false;
       if(ImGui::Checkbox("Random", &setting_.lifeTime.random))
           setting_.lifeTime.fixed = false;

        ImGui::NextColumn();
        ImGui::BeginDisabled(setting_.lifeTime.random);
        ImGui::Checkbox("Infinite", &parametor_.isInfiniteLife);
        ImGui::EndDisabled();

        ImGui::Columns(1);
        ImGui::SeparatorText("");

        if (parametor_.isInfiniteLife)
        {
            // 無限寿命だから設定できない余
            ImGui::TextDisabled(" Infinite LifeTime");
        }
        else if (setting_.lifeTime.fixed)
        {
            ImGui::SetNextItemWidth(width * 4);
            ImGui::DragFloat("Fix", &setting_.lifeTime.value.min, 0.01f, 0);
            setting_.lifeTime.value.max = setting_.lifeTime.value.min;
            ImGui::BeginDisabled(true);
            ImGui::SetNextItemWidth(width * 4);
            ImGui::DragFloat("Disabled", &setting_.lifeTime.value.max);
            ImGui::EndDisabled();
        }
        else
        {
            ImGui::SetNextItemWidth(width * 4);
            ImGui::DragFloat("Min", &setting_.lifeTime.value.min, 0.01f, 0.0f, setting_.lifeTime.value.max);
            ImGui::SetNextItemWidth(width * 4);
            ImGui::DragFloat("Max", &setting_.lifeTime.value.max, 0.01f, setting_.lifeTime.value.min);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
#endif // _DEBUG
}

void ParticleEmitter::DisplaySizeParameters()
{
#ifdef _DEBUG


    if (ImGui::TreeNodeEx("Size", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Columns(2, "chec", false);
        if (ImGui::Checkbox("fixed", &setting_.size.fixed))
            setting_.size.random = false;
        if (ImGui::Checkbox("Random",& setting_.size.random))
            setting_.size.fixed = false;

        ImGui::NextColumn();
        ImGui::Checkbox("Change", &parametor_.sizeTransition.isChange);

        ImGui::Columns(1);

        ImGui::SeparatorText("");

        ImGui::Text("Start Parameter");
        if (setting_.size.fixed)
        {
            ImGui::DragFloat3("Fix_Size", &setting_.size.value.min.x, 0.01f);
            setting_.size.value.max = setting_.size.value.min;
            ImGui::BeginDisabled(true);
            ImGui::DragFloat3("Disabled", &setting_.size.value.max.x);
            ImGui::EndDisabled();
        }
        else
        {
            ImGui::DragFloat3("Start", &setting_.size.value.min.x, 0.01f);
            ImGui::DragFloat3(" End ", &setting_.size.value.max.x, 0.01f);
        }

        if (parametor_.sizeTransition.isChange)
        {
            ImGui::SeparatorText("ChangeParameter");
            ImGui::DragFloat("Time", &addSize_.time, 0.01f, 0.01f, 0.99f);
            ImGui::DragFloat3("Size", &addSize_.value.x, 0.01f);

            addSize_.easingFuncNum = Easing::SelectEasingFunc();

            if (ImGui::Button("Save"))
            {
                addSize_.time = std::clamp(addSize_.time, 0.01f, 0.99f);

                if (parametor_.sizeTransition.keys.empty()) {
                    parametor_.sizeTransition.keys.emplace_back(addSize_);
                }
                else {
                    auto it = parametor_.sizeTransition.keys.begin();
                    for (; it != parametor_.sizeTransition.keys.end(); ++it) {
                        if (it->time >= addSize_.time) {
                            break;
                        }
                    }
                    parametor_.sizeTransition.keys.insert(it, addSize_);
                }
                addSize_ = {};
            }

            if (parametor_.sizeTransition.keys.size() != 0)
            {
                ImGui::SeparatorText("ChangeParameterList");
                static int selectedSizeKey_ = 0;
                int32_t index = 0;

                for (auto it = parametor_.sizeTransition.keys.begin(); it != parametor_.sizeTransition.keys.end();)
                {

                    std::string str =
                        "Time:" + std::to_string(it->time) + "##" +
                        std::to_string(std::distance(parametor_.sizeTransition.keys.begin(), it));

                    if (ImGui::Selectable(str.c_str(), selectedSizeKey_ == index))
                        selectedSizeKey_ = index;

                    if (ImGui::BeginPopupContextItem())
                    {
                        selectedSizeKey_ = index;

                        ImGui::Text("Time:%f", it->time);
                        ImGui::Text("Size:%f,%f,%f", it->value.x, it->value.y, it->value.z);
                        ImGui::Text("Func:%s", Easing::GetEasingFuncName(it->easingFuncNum).c_str());

                        if (ImGui::Button("Delete"))
                        {
                            it = parametor_.sizeTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        if (ImGui::Button("Edit"))
                        {
                            addSize_ = *it;
                            it = parametor_.sizeTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        ImGui::EndPopup();
                    }
                    ++it;
                    ++index;
                }
            }
        }
        ImGui::TreePop();
    }
#endif // _DEBUG

}

void ParticleEmitter::DisplayRotateParameters()
{
#ifdef _DEBUG

    float width = ImGui::GetContentRegionAvail().x / 5.0f; // 利用可能な幅を3等分
    ImGui::PushID("Rotate");
    if (ImGui::TreeNodeEx("Rotate", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Columns(2, "rotatechec", false);
        if (ImGui::Checkbox("Fixed", &setting_.rotate.fixed))
            setting_.rotate.random = false;
        if (ImGui::Checkbox("Random", &setting_.rotate.random))
            setting_.rotate.fixed = false;
        ImGui::NextColumn();
        ImGui::Checkbox("Change", &parametor_.rotateTransition.isChange);
        ImGui::Columns(1);
        ImGui::SeparatorText("");
        ImGui::PushItemWidth(width * 4);
        ImGui::Text("Start Parameter");
        if (setting_.rotate.fixed)
        {
            ImGui::DragFloat3("Fix", &setting_.rotate.value.min.x, 0.01f);
            setting_.rotate.value.max = setting_.rotate.value.min;
            ImGui::BeginDisabled(true);
            ImGui::DragFloat3("Disabled", &setting_.rotate.value.max.x);
            ImGui::EndDisabled();
        }
        else
        {
            ImGui::DragFloat3("Min", &setting_.rotate.value.min.x, 0.01f);
            ImGui::DragFloat3("Max", &setting_.rotate.value.max.x, 0.01f);
        }
        if (parametor_.rotateTransition.isChange)
        {
            ImGui::SeparatorText("ChangeParameter");
            ImGui::DragFloat("Time", &addRotate_.time, 0.01f, 0.01f, 0.99f);
            ImGui::DragFloat3("Rotate", &addRotate_.value.x, 0.01f);
            addRotate_.easingFuncNum = Easing::SelectEasingFunc();
            if (ImGui::Button("Save"))
            {
                addRotate_.time = std::clamp(addRotate_.time, 0.01f, 0.99f);
                if (parametor_.rotateTransition.keys.empty())
                {
                    parametor_.rotateTransition.keys.emplace_back(addRotate_);
                }
                else
                {
                    auto it = parametor_.rotateTransition.keys.begin();
                    for (; it != parametor_.rotateTransition.keys.end(); ++it)
                    {
                        if (it->time >= addRotate_.time)
                        {
                            break;
                        }
                    }
                    parametor_.rotateTransition.keys.insert(it, addRotate_);
                }
                addRotate_ = {};
            }
            if (parametor_.rotateTransition.keys.size() != 0)
            {
                ImGui::SeparatorText("ChangeParameterList");
                static int selectedRotateKey_ = 0;
                int32_t index = 0;
                for (auto it = parametor_.rotateTransition.keys.begin(); it != parametor_.rotateTransition.keys.end();)
                {
                    std::string str =
                        "Time:" + std::to_string(it->time) + "##" +
                        std::to_string(std::distance(parametor_.rotateTransition.keys.begin(), it));
                    if (ImGui::Selectable(str.c_str(), selectedRotateKey_ == index))
                        selectedRotateKey_ = index;
                    if (ImGui::BeginPopupContextItem())
                    {
                        selectedRotateKey_ = index;
                        ImGui::Text("Time:%f", it->time);
                        ImGui::Text("Rotate:%f,%f,%f", it->value.x, it->value.y, it->value.z);
                        ImGui::Text("Func:%s", Easing::GetEasingFuncName(it->easingFuncNum).c_str());
                        if (ImGui::Button("Delete"))
                        {
                            it = parametor_.rotateTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        if (ImGui::Button("Edit"))
                        {
                            addRotate_ = *it;
                            it = parametor_.rotateTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        ImGui::EndPopup();
                    }
                    ++it;
                    ++index;
                }
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();



#endif // _DEBUG

}

void ParticleEmitter::DisplaySpeedParameters()
{
#ifdef _DEBUG
    float width = ImGui::GetContentRegionAvail().x / 5.0f; // 利用可能な幅を3等分

    ImGui::PushID("Speed");
    if (ImGui::TreeNodeEx("Speed", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Columns(2, "speedchec", false);
        if(ImGui::Checkbox("Fixed", &setting_.speed.fixed))
            setting_.speed.random = false;
        if (ImGui::Checkbox("Random", &setting_.speed.random))
            setting_.speed.fixed = false;

        ImGui::NextColumn();
        ImGui::Checkbox("Change", &parametor_.speedTransition.isChange);
        ImGui::Columns(1);
        ImGui::SeparatorText("");

        ImGui::PushItemWidth(width * 4);

        ImGui::Text("Start Parameter");
        if (setting_.speed.fixed)
        {
            ImGui::DragFloat("Fix", &setting_.speed.value.min, 0.01f);
            setting_.speed.value.max = setting_.speed.value.min;
            ImGui::BeginDisabled(true);
            ImGui::DragFloat("Disabled", &setting_.speed.value.max);
            ImGui::EndDisabled();
        }
        else
        {
            ImGui::DragFloat("Min", &setting_.speed.value.min, 0.01f);
            ImGui::DragFloat("Max", &setting_.speed.value.max, 0.01f);
        }
        ImGui::PopItemWidth();

        if (parametor_.speedTransition.isChange)
        {
            ImGui::SeparatorText("ChangeParameter");
            ImGui::DragFloat("Time", &addSpeed_.time, 0.01f, 0.01f, 0.99f);
            ImGui::DragFloat("Speed", &addSpeed_.value, 0.01f);

            addSpeed_.easingFuncNum = Easing::SelectEasingFunc();

            if (ImGui::Button("Save"))
            {
                addSpeed_.time = std::clamp(addSpeed_.time, 0.01f, 0.99f);

                if (parametor_.speedTransition.keys.empty())
                {
                    parametor_.speedTransition.keys.emplace_back(addSpeed_);
                }
                else
                {
                    auto it = parametor_.speedTransition.keys.begin();
                    for (; it != parametor_.speedTransition.keys.end(); ++it)
                    {
                        if (it->time >= addSpeed_.time)
                        {
                            break;
                        }
                    }
                    parametor_.speedTransition.keys.insert(it, addSpeed_);
                }
                addSpeed_ = {};
            }

            if (parametor_.speedTransition.keys.size() != 0)
            {
                ImGui::SeparatorText("ChangeParameterList");
                static int selectedSpeedKey_ = 0;
                int32_t index = 0;
                for (auto it = parametor_.speedTransition.keys.begin(); it != parametor_.speedTransition.keys.end();)
                {
                    std::string str =
                        "Time:" + std::to_string(it->time) + "##" +
                        std::to_string(std::distance(parametor_.speedTransition.keys.begin(), it));

                    if (ImGui::Selectable(str.c_str(), selectedSpeedKey_ == index))
                        selectedSpeedKey_ = index;

                    if (ImGui::BeginPopupContextItem())
                    {
                        selectedSpeedKey_ = index;

                        ImGui::Text("Time:%f", it->time);
                        ImGui::Text("Speed:%f", it->value);
                        ImGui::Text("Func:%s", Easing::GetEasingFuncName(it->easingFuncNum).c_str());

                        if (ImGui::Button("Delete"))
                        {
                            it = parametor_.speedTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        if (ImGui::Button("Edit"))
                        {
                            addSpeed_ = *it;
                            it = parametor_.speedTransition.keys.erase(it);
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            continue;
                        }
                        ImGui::EndPopup();
                    }
                    ++it;
                    ++index;
                }
            }
        }

        ImGui::TreePop();
    }
    ImGui::PopID();
#endif // _DEBUG
}

void ParticleEmitter::DisplayDirectionParameters()
{
#ifdef _DEBUG

    if (ImGui::TreeNodeEx("Direction", ImGuiTreeNodeFlags_Framed))
    {
        float width = ImGui::GetContentRegionAvail().x / 5.0f; // 利用可能な幅を3等分

        /*ImGui::SeparatorText("Lock Axis");
        ImGui::SetNextItemWidth(width);
        ImGui::BeginDisabled(!lockRotationAxes_[0]);
        ImGui::DragFloat("X", &lockRotationAxesValue_.x, 0.01f);
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!lockRotationAxes_[1]);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat("Y", &lockRotationAxesValue_.y, 0.01f);
        ImGui::EndDisabled();
        ImGui::SameLine();

        ImGui::BeginDisabled(!lockRotationAxes_[2]);
        ImGui::SetNextItemWidth(width);
        ImGui::DragFloat("Z", &lockRotationAxesValue_.z, 0.01f);
        ImGui::EndDisabled();*/

        ImGui::PushID("direction");
        ImGui::BeginDisabled(particleDirection_ != ParticleDirection::Random);

        if(ImGui::Checkbox("Fixed", &setting_.direction.fixed))
        {
            setting_.direction.random = false;
            particleDirection_ = ParticleDirection::Random;
        }
        if (ImGui::Checkbox("Random", &setting_.direction.random))
        {
            setting_.direction.fixed = false;
        }


        ImGui::SeparatorText("");

        ImGui::Text("Start Parameter");

        if(particleDirection_ == ParticleDirection::Random)
        {
            if (setting_.direction.fixed)
            {
                ImGui::DragFloat3("Fix", &setting_.direction.value.min.x, 0.01f);
                setting_.direction.value.max = setting_.direction.value.min;
                ImGui::BeginDisabled(true);
                ImGui::DragFloat3("Disabled", &setting_.direction.value.max.x);
                ImGui::EndDisabled();
            }
            else
            {
                ImGui::DragFloat3("Min", &setting_.direction.value.min.x, 0.01f);
                ImGui::DragFloat3("Max", &setting_.direction.value.max.x, 0.01f);
            }
        }
        else
        {
            ImGui::DragFloat3("Min", &setting_.direction.value.min.x, 0.01f);
            ImGui::DragFloat3("Max", &setting_.direction.value.max.x, 0.01f);
        }
        ImGui::EndDisabled();
        ImGui::PopID();


        ImGui::TreePop();
    }

   /* for (size_t index = 0; index < 3; ++index)
    {
        if (lockRotationAxes_[index])
        {
            setting_.direction.value.min[index] = lockRotationAxesValue_[index];
            setting_.direction.value.max[index] = lockRotationAxesValue_[index];
        }
    }*/

#endif // _DEBUG

}

void ParticleEmitter::DisplayAccelerationParameters()
{
#ifdef _DEBUG

    ImGui::PushID("Acceleration");
   if( ImGui::TreeNodeEx("Acceleration",ImGuiTreeNodeFlags_Framed))
   {
       if (ImGui::Checkbox("Fixed", &setting_.acceleration.fixed))
           setting_.acceleration.random = false;
       if (ImGui::Checkbox("Random", &setting_.acceleration.random))
           setting_.acceleration.fixed = false;

       ImGui::SeparatorText("");

       ImGui::Text("Start Parameter");
       if (setting_.acceleration.fixed)
       {
           ImGui::DragFloat3("Fix", &setting_.acceleration.value.min.x, 0.01f);
           setting_.acceleration.value.max = setting_.acceleration.value.min;
           ImGui::BeginDisabled(true);
           ImGui::DragFloat3("Disabled", &setting_.acceleration.value.max.x);
           ImGui::EndDisabled();
       }
       else
       {
           ImGui::DragFloat3("Min", &setting_.acceleration.value.min.x, 0.01f);
           ImGui::DragFloat3("Max", &setting_.acceleration.value.max.x, 0.01f);
       }
       ImGui::TreePop();
   }

    ImGui::PopID();
#endif // _DEBUG
}

void ParticleEmitter::DisplayColorParameters()
{
#ifdef _DEBUG
    ImGui::PushID("Color");
    if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_Framed))
    {
        ImGui::Columns(2, "colorChec", false);
        if (ImGui::Checkbox("Fixed", &setting_.color.fixed))
            setting_.color.random = false;
        if (ImGui::Checkbox("Random", &setting_.color.random))
            setting_.color.fixed = false;

        ImGui::NextColumn();
        ImGui::Checkbox("Change", &parametor_.colorTransition.isChange);
        ImGui::Columns(1);


        ImGui::SeparatorText("");
        ImGui::Text("Start Parameter");
        if (setting_.color.fixed)
        {
            ImGui::ColorEdit4("Fix", &setting_.color.value.min.x);
            setting_.color.value.max = setting_.color.value.min;
            ImGui::BeginDisabled(true);
            ImGui::ColorEdit4("Disabled", &setting_.color.value.max.x);
            ImGui::EndDisabled();
        }
        else if (!setting_.color.random)
        {
            ImGui::ColorEdit4("Min", &setting_.color.value.min.x);
            ImGui::ColorEdit4("Max", &setting_.color.value.max.x);
        }

        if (parametor_.colorTransition.isChange)
        {
            ImGui::SeparatorText("ChangeParameter");
            if (ImGui::TreeNode("RGB"))
            {
                ImGui::DragFloat("Time", &addColor_.time, 0.01f, 0.01f, 0.99f);
                ImGui::ColorEdit3("Color", &addColor_.value.x);
                if (ImGui::Button("Save"))
                {
                    if (parametor_.colorTransition.keys.empty())
                    {
                        parametor_.colorTransition.keys.emplace_back(addColor_);
                    }
                    else
                    {
                        auto it = parametor_.colorTransition.keys.begin();
                        for (; it != parametor_.colorTransition.keys.end(); ++it)
                        {
                            if (it->time >= addColor_.time)
                            {
                                break;
                            }
                        }
                        parametor_.colorTransition.keys.insert(it, addColor_);
                    }
                }

                if (parametor_.colorTransition.keys.size() != 0)
                {
                    ImGui::SeparatorText("ChangeParameterList");
                    static int selectedColorKey_ = 0;
                    int32_t index = 0;
                    for (auto it = parametor_.colorTransition.keys.begin(); it != parametor_.colorTransition.keys.end();)
                    {
                        std::string str =
                            "Time:" + std::to_string(it->time) + "##" +
                            std::to_string(std::distance(parametor_.colorTransition.keys.begin(), it));

                        if (ImGui::Selectable(str.c_str(), selectedColorKey_ == index))
                            selectedColorKey_ = index;
                        if (ImGui::BeginPopupContextItem())
                        {
                            selectedColorKey_ = index;
                            ImGui::Text("Time:%f", it->time);
                            ImGui::Text("Color:%f,%f,%f", it->value.x, it->value.y, it->value.z);
                            if (ImGui::Button("Delete"))
                            {
                                it = parametor_.colorTransition.keys.erase(it);
                                ImGui::CloseCurrentPopup();
                                ImGui::EndPopup();
                                continue;
                            }
                            if (ImGui::Button("Edit"))
                            {
                                addColor_ = *it;
                                it = parametor_.colorTransition.keys.erase(it);
                                ImGui::CloseCurrentPopup();
                                ImGui::EndPopup();
                                continue;
                            }
                            ImGui::EndPopup();
                        }
                        ++it;
                        ++index;
                    }
                }

                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Alpha"))
            {
                ImGui::DragFloat("Time", &addAlpha_.time, 0.01f, 0.01f, 0.99f);
                ImGui::DragFloat("Alpha", &addAlpha_.value, 0.01f);
                if (ImGui::Button("Save"))
                {
                    if (parametor_.alphaTransition.keys.empty())
                    {
                        parametor_.alphaTransition.keys.emplace_back(addAlpha_);
                    }
                    else
                    {
                        auto it = parametor_.alphaTransition.keys.begin();
                        for (; it != parametor_.alphaTransition.keys.end(); ++it)
                        {
                            if (it->time >= addAlpha_.time)
                            {
                                break;
                            }
                        }
                        parametor_.alphaTransition.keys.insert(it, addAlpha_);
                    }
                }

                if (parametor_.alphaTransition.keys.size() != 0)
                {
                    ImGui::SeparatorText("ChangeParameterList");
                    static int selectedAlphaKey_ = 0;
                    int32_t index = 0;
                    for (auto it = parametor_.alphaTransition.keys.begin(); it != parametor_.alphaTransition.keys.end();)
                    {
                        std::string str =
                            "Time:" + std::to_string(it->time) + "##" +
                            std::to_string(std::distance(parametor_.alphaTransition.keys.begin(), it));

                        if (ImGui::Selectable(str.c_str(), selectedAlphaKey_ == index))
                            selectedAlphaKey_ = index;
                        if (ImGui::BeginPopupContextItem())
                        {
                            selectedAlphaKey_ = index;
                            ImGui::Text("Time:%f", it->time);
                            ImGui::Text("Alpha:%f", it->value);
                            if (ImGui::Button("Delete"))
                            {
                                it = parametor_.alphaTransition.keys.erase(it);
                                ImGui::CloseCurrentPopup();
                                ImGui::EndPopup();
                                continue;
                            }
                            if (ImGui::Button("Edit"))
                            {
                                addAlpha_ = *it;
                                it = parametor_.alphaTransition.keys.erase(it);
                                ImGui::CloseCurrentPopup();
                                ImGui::EndPopup();
                                continue;
                            }
                            ImGui::EndPopup();
                        }
                        ++it;
                        ++index;
                    }
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
#endif // _DEBUG
}

void ParticleEmitter::DisplayFlags()
{
#ifdef _DEBUG


    ImGui::Columns(2, "mycolumns", false);
    ImGui::Checkbox("loop", &loop_);
    ImGui::BeginDisabled(isLengthScalingEnabled_);
    if (ImGui::Checkbox("useBillboard", &isEnableBillboard_))
        isLengthScalingEnabled_ = false;
    ImGui::EndDisabled();
    ImGui::BeginDisabled(isEnableBillboard_);
    if (ImGui::Checkbox("shouldFaceDirection", &isLengthScalingEnabled_))
        isEnableBillboard_ = false;
    ImGui::EndDisabled();


    ImGui::Columns(1);

   /* ImGui::SeparatorText("Lock Axis");
    ImGui::Checkbox("X", &lockRotationAxes_[0]);
    ImGui::SameLine();
    ImGui::Checkbox("Y", &lockRotationAxes_[1]);
    ImGui::SameLine();
    ImGui::Checkbox("Z", &lockRotationAxes_[2]);*/

    ImGui::BeginDisabled(!isEnableBillboard_);
    ImGui::SeparatorText("use billboard");
    ImGui::PushID("billboard");

    std::array<bool, 3> axes;
    axes[0] = billboardAxes_.x == 0 ? false : true;
    axes[1] = billboardAxes_.y == 0 ? false : true;
    axes[2] = billboardAxes_.z == 0 ? false : true;

    ImGui::Checkbox("X", &axes[0]);
    ImGui::SameLine();
    ImGui::Checkbox("Y", &axes[1]);
    ImGui::SameLine();
    ImGui::Checkbox("Z", &axes[2]);
    ImGui::PopID();
    ImGui::EndDisabled();

#endif // _DEBUG
}
