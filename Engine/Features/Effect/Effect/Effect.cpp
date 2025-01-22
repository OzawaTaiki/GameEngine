#include <Features/Effect/Effect/Effect.h>
#include <System/Time/Time.h>
#include <Features/Effect/Manager/ParticleManager.h>

Effect::~Effect()
{
    for (auto& emitter : emitters_)
    {
        emitter.reset();
    }
    emitters_.clear();
}

void Effect::Initialize(const std::string& _name)
{
    name_ = _name;

    jsonBinder_ = std::make_unique<JsonBinder>(_name, "Resources/Data/Particles/Effects/");
    jsonBinder_->SetFolderPath();

    jsonBinder_->RegisterVariable("loop", reinterpret_cast<uint32_t*>(&isLoop_));
    jsonBinder_->RegisterVariable("emitters", &emitterNames_);

    //config_ = std::make_unique<Config>(name_,"Resources/Data/Particles/Effects/");
    //instance->SetDirectoryPath("resources/Data/Particles/Effects");

    //config_->SetVariable( "loop", reinterpret_cast<uint32_t*>(&isLoop_));
    //config_->SetVariable( "emitters", &emitterNames_);

    for (std::string emitterName : emitterNames_)
    {
        emitters_.emplace_back(std::make_unique<ParticleEmitter>());
        emitters_.back()->Initialize(emitterName);
    }

    isActive_ = false;

    gameTime_ = GameTime::GetInstance();
}

void Effect::Update()
{
    if (!isActive_)
    {
        Reset();
        return;
    }

    elapsedTime_ += gameTime_->GetChannel(timeChannel_).GetDeltaTime<float>();
    //elapsedTime_ += Time::GetDeltaTime<float>();

    // isActive_制御用
    bool active = false;

    for (auto& emitter : emitters_)
    {
        // 遅延時間が経過していなかったらスキップ
        if (elapsedTime_ <= emitter->GetDelayTime())
        {
            emitter->SetActive(false);
            continue;
        }
        if (elapsedTime_ >= emitter->GetDuration() + emitter->GetDelayTime())
        {
            emitter->SetAlive(false);
        }


        if (!emitter->IsActive() && emitter->IsAlive())
            emitter->SetActive(true);

        // 未だ有効なエミッターがあるときフラグを立てる
        if(emitter->IsActive() && !emitter->IsAlive())
            active = true;
        // 更新
        emitter->Update();
    }
    // 一個でもエミッターが有効なら true
    // そうじゃなければ false
    isActive_ = active;
}

void Effect::AddEmitter(const std::string& _name)
{
    if (emitterNames_.size() < kMaxEmitters)
        emitterNames_.emplace_back(_name);
    else
        return ;


    emitters_.emplace_back(std::make_unique<ParticleEmitter>());

    emitters_.back()->Initialize(_name);
}

std::list<ParticleEmitter*> Effect::GetEmitters() const
{
    std::list<ParticleEmitter*> list;

    for (auto& emitter : emitters_)
    {
        list.emplace_back(const_cast<ParticleEmitter*>(emitter.get()));
    }

    return list;
}

void Effect::SetActive(bool _active)
{
    isActive_ = _active;
    Reset();
}

void Effect::ExclusionEmitter(const std::string& _name)
{
    for (auto it = emitters_.begin(); it != emitters_.end();)
    {
        if ((*it)->GetName() == _name)
        {
            it = emitters_.erase(it);
            auto nameIt = std::find(emitterNames_.begin(), emitterNames_.end(), _name);
            emitterNames_.erase(nameIt);
            return;
        }
        ++it;
    }
}

void Effect::DebugShowForEmitterCreate()
{
#ifdef _DEBUG

    // stringをcharに変換
    if (ImGui::InputText("EmitterName", emitterBuf_, 256))
        addEmitterName_ = emitterBuf_;


    ImGui::BeginDisabled(strcmp(emitterBuf_, "") == 0);
    // エミッターの追加
    if (ImGui::Button("Create New Emitter"))
    {
        addEmitterName_ = emitterBuf_;
        AddEmitter(addEmitterName_);

        strcpy_s(emitterBuf_, sizeof(emitterBuf_), "");
        addEmitterName_ = "";
    }
    ImGui::EndDisabled();

    //エミッターの名前一覧
    static bool isSelect[kMaxEmitters];
    if (ImGui::TreeNode("Emitters"))
    {
        size_t index = 0;
        for (auto& emitter : emitters_)
        {
            ImGui::Selectable(emitter->GetName().c_str(), &isSelect[index]);
            ++index;
        }

        ImGui::TreePop();
    }

    int cnt = 0;
    for (auto& emitter : emitters_)
    {
        if (isSelect[cnt++])
        {
            if (emitter->ShowDebugWinsow())
            {
                if (ImGui::Button("Exclusion"))
                {
                    ParticleManager::GetInstance()->ClearGroup(emitter->GetName());
                    ExclusionEmitter(emitter->GetName());
                    emitters_.remove(emitter);
                    break;
                }
            }
        }
    }
#endif // _DEBUG
}

void Effect::SetTimeChannel(const std::string& _channel)
{
    timeChannel_ = _channel;

    for (auto& emitter : emitters_)
    {
        emitter->SetTimeChannel(_channel);
    }
}


void Effect::Save() const
{
    jsonBinder_->Save();

    for (auto& emitter : emitters_)
    {
        emitter->Save();
    }
}

void Effect::Reset()
{
    elapsedTime_ = 0;
    for (auto& emitter : emitters_)
    {
        emitter->Reset();
    }
}
