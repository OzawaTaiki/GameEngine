#include "ParticleEmitters.h"
#include "ParticleManager.h"
#include "RandomGenerator.h"
#include "LineDrawer.h"
#include "MatrixFunction.h"
#include <imgui.h>

void ParticleEmitter::Setting(const Vector3& _center, const Vector3& _rotate, uint32_t _countPerEmit, uint32_t _emitPerSec, uint32_t _maxParticle, bool _randomColor)
{
    position_ = _center;
    rotate_ = _rotate;
    countPerEmit_ = _countPerEmit;
    emitPerSec_ = _emitPerSec;
    maxParticles_ = _maxParticle;
    randomColor_ = _randomColor;

    setting_.acceleration = { {0,0,0},{0,0,0} };
    setting_.color = { {1,1,1,1},{1,1,1,1} };
    setting_.direction = { {-1,-1,-1},{1,1,1} };
    setting_.lifeTime = { 1,3 };
    setting_.rotate = { {0,0,0} ,{0,0,0} };
    setting_.size = { {1,1,1},{1,1,1} };
    setting_.spped = { 0.1f,0.5f };

    emitTime_ = 1.0f / static_cast<float> (emitPerSec_);
}

void ParticleEmitter::Update()
{
    currentTime_ += deltaTime_;
    //if()
    ImGui::Begin("emit");

    //for (int i = 0; i < 5; i++)
    //{
    //    std::string num = std::to_string(i);
    //    std::string str = "nasu" + num;
    //    ImGui::BeginTabBar("nasu");
    //    if(ImGui::BeginTabItem(str.c_str())){
    //        ImGui::DragFloat("nasu", &setting_.spped.min);
    //        ImGui::EndTabItem();
    //    }
    //    ImGui::EndTabBar();
    //}

    if (emitTime_ <= currentTime_)
    {
        std::vector<Particle> particles;

        for (uint32_t count = 0; count < countPerEmit_; ++count)
        {
            particles.push_back(GenerateParticleData());
        }

        ParticleManager::GetInstance()->AddParticleToGroup("sample", particles);
        currentTime_ = 0;
    }

    if (ImGui::Button("add"))
    {
        std::vector<Particle> particles;

        for (uint32_t count = 0; count < countPerEmit_; ++count)
        {
            particles.push_back(GenerateParticleData());
        }

        ParticleManager::GetInstance()->AddParticleToGroup("sample", particles);
        currentTime_ = 0;
    }
    ImGui::End();
}

void ParticleEmitter::Draw()
{
    switch (shape_)
    {
    case EmitterShape::Box:
        {
            Matrix4x4 affine = MakeAffineMatrix(size_, rotate_, position_);
            LineDrawer::GetInstance()->DrawOBB(affine);
        }
        break;
    case EmitterShape::Shpere:
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

Particle ParticleEmitter::GenerateParticleData()
{
    Particle particle;

    auto random = RandomGenerator::GetInstance();

    float lifeTIme          = random->GetUniformFloat   (setting_.lifeTime.min      , setting_.lifeTime.max     );
    Vector3 size            = random->GetUniformVec3    (setting_.size.min          , setting_.size.max         );
    Vector3 rotate          = random->GetUniformVec3    (setting_.rotate.min        , setting_.rotate.max       );
    float speed             = random->GetUniformFloat   (setting_.spped.min         , setting_.spped.max        );
    Vector3 direction       = random->GetUniformVec3    (setting_.direction.min     , setting_.direction.max    );
    Vector3 acceleration    = random->GetUniformVec3    (setting_.acceleration.min  , setting_.acceleration.max );


    Vector4 color;
    if (randomColor_)
        color = random->GetUniformColor();
    else
        color = random->GetUniformVec4(setting_.color.min, setting_.color.max);


    Vector3 pos{};
    switch (shape_)
    {
    case EmitterShape::Box:
        {
            Vector3 halfSize = size_ / 2.0f;
            pos = random->GetUniformVec3(-halfSize, halfSize);
        }
        break;
    case EmitterShape::Shpere:
        {
            float rad = random->GetUniformAngle();
            pos.x = std::cosf(rad);
            pos.y = random->GetUniformFloat(-radius_, radius_);
            pos.z = std::sinf(rad);
        }
        break;
    case EmitterShape::Circle:
        {
            float rad = random->GetUniformAngle();
            pos.x = std::cosf(rad);
            pos.y = 0;
            pos.z = std::sinf(rad);
        }
        break;
    case EmitterShape::None:
    default:
        throw std::runtime_error("Unknown Emitter Shape");
        break;
    }

    pos += position_;

    particle.Initialize(lifeTIme, size, rotate, pos, color, speed, direction, acceleration);
    return particle;
}