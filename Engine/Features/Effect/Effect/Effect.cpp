#include "Effect.h"
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Model/Manager/ModelManager.h>
#include <Debug/ImguITools.h>
#include <Math/Matrix/MatrixFunction.h>

#include <filesystem>
#include <algorithm>
#include <chrono>


namespace Engine {

Effect::~Effect()
{
    ClearEmitters();
}

std::unique_ptr<Effect> Effect::Create(const std::string& _name)
{
    auto effect = std::make_unique<Effect>();
    if (effect->Initialize(_name))
    {
        return effect;
    }
    return nullptr;
}

std::unique_ptr<Effect> Effect::CreateFromFile(std::string_view _filePath)
{
    // ファイル存在確認
    if (!std::filesystem::exists(_filePath))
    {
        return nullptr;
    }

    // ファイル名からエフェクト名を抽出
    std::string name = std::filesystem::path(_filePath).stem().string();

    auto effect = std::make_unique<Effect>();
    if (effect->LoadFromFile(_filePath))
    {
        return effect;
    }
    return nullptr;
}

bool Effect::Initialize(const std::string& _name)
{
    if (_name.empty())
    {
        SetError("Effect name cannot be empty");
        return false;
    }

    ClearError();
    name_ = _name;

    try
    {
        jsonBinder_ = std::make_unique<JsonBinder>(_name, "Resources/Data/Particles/Effects/");

        jsonBinder_->RegisterVariable("loop", reinterpret_cast<uint32_t*>(&isLoop_));
        jsonBinder_->RegisterVariable("emitters", &emitterNames_);
        jsonBinder_->RegisterVariable("playbackSpeed", &playbackSpeed_);
        jsonBinder_->RegisterVariable("startDelay", &startDelay_);
        jsonBinder_->RegisterVariable("position", &position_);
        jsonBinder_->RegisterVariable("rotation", &rotation_);
        jsonBinder_->RegisterVariable("scale", &scale_);
        jsonBinder_->RegisterVariable("renderLayer", &renderLayer_);

        // 既存のエミッターを読み込み
        for (const std::string& emitterName : emitterNames_)
        {
            if (!AddEmitter(emitterName))
            {
                SetError("Failed to load emitter: " + emitterName);
                // 警告として続行
            }
        }

        gameTime_ = GameTime::GetInstance();

        isActive_ = false;
        isPaused_ = false;
        elapsedTime_ = 0.0f;

        RebuildIndexMap();

        return true;
    }
    catch (const std::exception& e)
    {
        SetError("Initialization failed: " + std::string(e.what()));
        return false;
    }
}

void Effect::Play()
{
    Reset();
    isActive_ = true;
    isPaused_ = false;
    InvokeCallback(onPlayCallback_);
}

void Effect::Stop()
{
    isActive_ = false;
    isPaused_ = false;
    InvokeCallback(onStopCallback_);
}

void Effect::Pause()
{
    if (isActive_)
    {
        isPaused_ = true;
    }
}

void Effect::Resume()
{
    if (isActive_)
    {
        isPaused_ = false;
    }
}

void Effect::Reset()
{
    elapsedTime_ = 0.0f;
    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->Reset();
        }
    }
    ResetStatistics();
}

bool Effect::IsComplete() const
{
    if (!isActive_ || isLoop_) return false;

    // 全エミッターが非アクティブかつ寿命切れの場合完了
    for (const auto& emitter : emitters_)
    {
        if (emitter && (emitter->IsActive() || emitter->IsAlive()))
        {
            return false;
        }
    }
    return true;
}

void Effect::Update()
{
    if (!isActive_ || isPaused_)
    {
        return;
    }

    if (!gameTime_)
    {
        SetError("GameTime not initialized");
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    float deltaTime = gameTime_->GetChannel(timeChannel_).GetDeltaTime<float>() * playbackSpeed_;
    elapsedTime_ += deltaTime;

    // 開始遅延チェック
    if (elapsedTime_ < startDelay_)
    {
        return;
    }

    UpdateEmitters();

    // 完了チェック
    if (IsComplete())
    {
        isActive_ = false;
        InvokeCallback(onCompleteCallback_);
    }
    else if (isLoop_ && elapsedTime_ >= GetDuration())
    {
        // ループ時のリセット
        Reset();
        isActive_ = true;
        InvokeCallback(onLoopCallback_);
    }

    // 統計更新
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    statistics_.averageUpdateTime = duration.count() / 1000.0f; // ms

}

void Effect::Draw()
{
    if (!isActive_ || !isVisible_)
    {
        return;
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // ワールド行列更新
    UpdateWorldMatrix();

    uint32_t drawCalls = 0;
    for (auto& emitter : emitters_)
    {
        if (emitter && emitter->IsActive())
        {
            // emitter->Draw(); // ParticleEmitterにDraw()メソッドがあると仮定
            drawCalls++;
        }
    }

    // 統計更新
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    statistics_.averageDrawTime = duration.count() / 1000.0f; // ms
    statistics_.drawCalls = drawCalls;

#ifdef _DEBUG
    if (enableDebugDraw_)
    {
        DebugDrawBounds();
        DebugDrawEmitters();
    }
#endif
}

void Effect::DrawShadow()
{
    if (!isActive_ || !isVisible_)
    {
        return;
    }

    for (auto& emitter : emitters_)
    {
        if (emitter && emitter->IsActive())
        {
            // emitter->DrawShadow(); // ParticleEmitterにDrawShadow()メソッドがあると仮定
        }
    }
}

bool Effect::AddEmitter(const std::string& _name)
{
    if (_name.empty())
    {
        SetError("Emitter name cannot be empty");
        return false;
    }

    if (emitters_.size() >= kMaxEmitters)
    {
        SetError("Maximum number of emitters reached");
        return false;
    }

    // 重複チェック
    if (HasEmitter(_name))
    {
        SetError("Emitter already exists: " + _name);
        return false;
    }

    try
    {
        auto emitter = std::make_unique<ParticleEmitter>();
        if (!emitter->Initialize(_name))
        {
            SetError("Failed to initialize emitter: " + emitter->GetLastError());
            return false;
        }

        // 位置設定
        emitter->SetPosition(position_);
        emitter->SetTimeChannel(timeChannel_);

        emitters_.emplace_back(std::move(emitter));
        emitterNames_.emplace_back(_name);

        RebuildIndexMap();
        ClearError();
        return true;
    }
    catch (const std::exception& e)
    {
        SetError("Failed to create emitter: " + std::string(e.what()));
        return false;
    }
}

bool Effect::CreateEmitter(const std::string& _name, const std::string& _templateName)
{
    if (!AddEmitter(_name))
    {
        return false;
    }

    // テンプレートが指定されている場合は設定をコピー
    if (!_templateName.empty())
    {
        ParticleEmitter* templateEmitter = FindEmitter(_templateName);
        ParticleEmitter* newEmitter = FindEmitter(_name);

        if (templateEmitter && newEmitter)
        {
            // テンプレートから設定をコピー
            // 実装が必要: newEmitter->CopySettingsFrom(templateEmitter);
        }
    }

    return true;
}

bool Effect::RemoveEmitter(std::string_view _name)
{
    auto it = std::find_if(emitters_.begin(), emitters_.end(),
        [_name](const std::unique_ptr<ParticleEmitter>& _emitter) {
            return _emitter && _emitter->GetName() == _name;
        });

    if (it != emitters_.end())
    {
        emitters_.erase(it);

        auto nameIt = std::find_if(emitterNames_.begin(), emitterNames_.end(),
            [_name](const std::string& _emitterName) {
                return _emitterName == _name;
            });

        if (nameIt != emitterNames_.end())
        {
            emitterNames_.erase(nameIt);
        }

        RebuildIndexMap();
        ClearError();
        return true;
    }

    SetError("Emitter not found: " + std::string(_name));
    return false;
}

ParticleEmitter* Effect::FindEmitter(std::string_view _name) const
{
    auto it = std::find_if(emitters_.begin(), emitters_.end(),
        [_name](const std::unique_ptr<ParticleEmitter>& _emitter) {
            return _emitter && _emitter->GetName() == _name;
        });

    return (it != emitters_.end()) ? it->get() : nullptr;
}

bool Effect::HasEmitter(std::string_view _name) const
{
    return FindEmitter(_name) != nullptr;
}

void Effect::ClearEmitters()
{
    emitters_.clear();
    emitterNames_.clear();
    emitterIndexMap_.clear();
}

std::list<ParticleEmitter*> Effect::GetEmitters() const
{
    std::list<ParticleEmitter*> result;
    for (const auto& emitter : emitters_)
    {
        if (emitter)
        {
            result.emplace_back(emitter.get());
        }
    }
    return result;
}

std::vector<std::string> Effect::GetEmitterNames() const
{
    return emitterNames_;
}

ParticleEmitter* Effect::GetEmitterAt(size_t _index) const
{
    if (_index >= emitters_.size()) return nullptr;

    auto it = emitters_.begin();
    std::advance(it, _index);
    return it->get();
}

void Effect::SetName(const std::string& _name)
{
    if (!_name.empty())
    {
        name_ = _name;
        ClearError();
    }
    else
    {
        SetError("Name cannot be empty");
    }
}

void Effect::SetTimeChannel(const std::string& _channel)
{
    timeChannel_ = _channel;
    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->SetTimeChannel(_channel);
        }
    }
}

void Effect::SetParentMatrix(const Matrix4x4* _parentMat)
{
    if (_parentMat)
    {
        worldMatrix_ = *_parentMat;
    }
    else
    {
        UpdateWorldMatrix();
    }

    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            // emitter->SetParentMatrix(&worldMatrix_); // 必要に応じて実装
        }
    }
}

void Effect::SetPosition(const Vector3& _position)
{
    position_ = _position;
    UpdateWorldMatrix();

    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->SetPosition(_position);
        }
    }
}

void Effect::SetRotation(const Quaternion& _rotation)
{
    rotation_ = _rotation;
    UpdateWorldMatrix();
}

void Effect::SetScale(const Vector3& _scale)
{
    scale_ = _scale;
    UpdateWorldMatrix();
}

float Effect::GetDuration() const
{
    float maxDuration = 0.0f;
    for (const auto& emitter : emitters_)
    {
        if (emitter)
        {
            float emitterDuration = emitter->GetDelayTime() + emitter->GetDuration();
            maxDuration = (std::max)(maxDuration, emitterDuration);
        }
    }
    return maxDuration;
}

float Effect::GetProgress() const
{
    float duration = GetDuration();
    if (duration <= 0.0f) return 1.0f;

    return (std::min)(1.0f, elapsedTime_ / duration);
}

bool Effect::LoadFromFile(std::string_view _filePath)
{
    try
    {
        std::string name = std::filesystem::path(_filePath).stem().string();
        if (!Initialize(name))
        {
            return false;
        }

        // JsonBinderから設定を読み込み
        if (jsonBinder_)
        {
            // jsonBinder_->LoadFromFile(_filePath); // 実装が必要
        }

        ClearError();
        return true;
    }
    catch (const std::exception& e)
    {
        SetError("Load failed: " + std::string(e.what()));
        return false;
    }
}

bool Effect::SaveToFile() const
{
    try
    {
        if (!jsonBinder_)
        {
            SetError("JsonBinder not initialized");
            return false;
        }

        jsonBinder_->Save();

        // 各エミッターも保存
        for (const auto& emitter : emitters_)
        {
            if (emitter)
            {
                emitter->SaveToFile();  // エミッター固有のパスで保存
            }
        }

        ClearError();
        return true;
    }
    catch (const std::exception& e)
    {
        SetError("Save failed: " + std::string(e.what()));
        return false;
    }
}

bool Effect::Save() const
{
    return SaveToFile();
}


void Effect::SetAllEmittersActive(bool _active)
{
    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->SetActive(_active);
        }
    }
}

void Effect::SetEmitterGroupActive(std::string_view _groupName, bool _active)
{
    for (auto& emitter : emitters_)
    {
        if (emitter && emitter->GetName().find(_groupName) != std::string::npos)
        {
            emitter->SetActive(_active);
        }
    }
}

void Effect::ScaleAllEmitters(float _scale)
{
    Vector3 scaleVec(_scale, _scale, _scale);
    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->SetScale(emitter->GetScale() * scaleVec);
        }
    }
}


void Effect::UpdateWorldMatrix()
{
    // ワールド行列
    worldMatrix_ = MakeAffineMatrix(scale_, rotation_, position_);
}

void Effect::UpdateEmitters()
{
    if (!gameTime_) return;

    float deltaTime = gameTime_->GetChannel(timeChannel_).GetDeltaTime<float>() * playbackSpeed_;

    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitter->Update(deltaTime);
        }
    }
}

bool Effect::ValidateEmitterName(std::string_view _name) const
{
    if (_name.empty()) return false;
    if (_name.length() > 255) return false;

    // 無効な文字のチェック
    const std::string invalidChars = "<>:\"/\\|?*";
    for (char c : _name)
    {
        if (invalidChars.find(c) != std::string::npos)
        {
            return false;
        }
    }

    return true;
}

void Effect::RebuildIndexMap()
{
    emitterIndexMap_.clear();
    size_t index = 0;
    for (const auto& emitter : emitters_)
    {
        if (emitter)
        {
            emitterIndexMap_[emitter->GetName()] = index;
        }
        ++index;
    }
}

void Effect::InvokeCallback(EffectCallback& _callback)
{
    if (_callback)
    {
        try
        {
            _callback(this);
        }
        catch (const std::exception& e)
        {
            SetError("Callback error: " + std::string(e.what()));
        }
    }
}

#ifdef _DEBUG
void Effect::ShowDebugWindow()
{
    ImGui::Begin(("Effect: " + name_).c_str());

    // 基本情報
    if (ImGui::CollapsingHeader("Basic Info"))
    {
        ImGui::Text("Name: %s", name_.c_str());
        ImGui::Text("Active: %s", isActive_ ? "Yes" : "No");
        ImGui::Text("Paused: %s", isPaused_ ? "Yes" : "No");
        ImGui::Text("Loop: %s", isLoop_ ? "Yes" : "No");
        ImGui::Text("Elapsed Time: %.3f", elapsedTime_);
        ImGui::Text("Duration: %.3f", GetDuration());
        ImGui::Text("Progress: %.1f%%", GetProgress() * 100.0f);

        ImGui::Separator();

        if (ImGui::Button("Play")) Play();
        ImGui::SameLine();
        if (ImGui::Button("Stop")) Stop();
        ImGui::SameLine();
        if (ImGui::Button("Pause")) Pause();
        ImGui::SameLine();
        if (ImGui::Button("Resume")) Resume();
        ImGui::SameLine();
        if (ImGui::Button("Reset")) Reset();
    }

    // 設定
    if (ImGui::CollapsingHeader("Settings"))
    {
        ImGui::Checkbox("Loop", &isLoop_);
        ImGui::Checkbox("Visible", &isVisible_);
        ImGui::Checkbox("Debug Draw", &enableDebugDraw_);

        ImGui::DragFloat("Playback Speed", &playbackSpeed_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("Start Delay", &startDelay_, 0.01f, 0.0f, 10.0f);

        int layer = static_cast<int>(renderLayer_);
        if (ImGui::InputInt("Render Layer", &layer))
        {
            renderLayer_ = static_cast<uint32_t>((std::max)(0, layer));
        }

        ImGui::Separator();

        ImGui::DragFloat3("Position", &position_.x, 0.01f);
        ImGui::DragFloat4("Rotation", &rotation_.x, 0.01f);
        ImGui::DragFloat3("Scale", &scale_.x, 0.01f);
    }

    // エミッター管理
    if (ImGui::CollapsingHeader("Emitters"))
    {
        ImGui::InputText("New Emitter", newEmitterNameBuf_, sizeof(newEmitterNameBuf_));
        ImGui::SameLine();
        if (ImGui::Button("Add"))
        {
            if (strlen(newEmitterNameBuf_) > 0)
            {
                AddEmitter(std::string(newEmitterNameBuf_));
                memset(newEmitterNameBuf_, 0, sizeof(newEmitterNameBuf_));
            }
        }

        ImGui::Separator();

        if (ImGui::BeginChild("EmitterList", ImVec2(0, 200)))
        {
            size_t index = 0;
            for (auto& emitter : emitters_)
            {
                if (emitter)
                {
                    ImGui::PushID(static_cast<int>(index));

                    bool active = emitter->IsActive();
                    if (ImGui::Checkbox("##Active", &active))
                    {
                        emitter->SetActive(active);
                    }

                    ImGui::SameLine();
                    if (ImGui::Selectable(emitter->GetName().c_str()))
                    {
                        // エミッター選択処理
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Remove"))
                    {
                        RemoveEmitter(emitter->GetName());
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }
                ++index;
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        if (ImGui::Button("Clear All"))
        {
            ClearEmitters();
        }
        ImGui::SameLine();
        if (ImGui::Button("Activate All"))
        {
            SetAllEmittersActive(true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Deactivate All"))
        {
            SetAllEmittersActive(false);
        }
    }

    // 統計
    ShowStatistics();

    // エラー表示
    if (HasError())
    {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", GetLastError().c_str());
        if (ImGui::Button("Clear Error"))
        {
            ClearError();
        }
    }

    ImGui::End();
}

void Effect::ShowEmitterList()
{
    ImGui::Begin(("Emitters - " + name_).c_str());

    for (auto& emitter : emitters_)
    {
        if (emitter)
        {
            if (ImGui::TreeNode(emitter->GetName().c_str()))
            {
                emitter->ShowDebugWindow();
                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void Effect::ShowStatistics()
{
    if (ImGui::CollapsingHeader("Statistics"))
    {
        ImGui::Text("Total Particles: %u", statistics_.totalParticles);
        ImGui::Text("Active Particles: %u", statistics_.activeParticles);
        ImGui::Text("Update Time: %.3f ms", statistics_.averageUpdateTime);
        ImGui::Text("Draw Time: %.3f ms", statistics_.averageDrawTime);
        ImGui::Text("Draw Calls: %u", statistics_.drawCalls);
        ImGui::Text("Emitter Count: %zu", emitters_.size());

        if (ImGui::Button("Reset Statistics"))
        {
            ResetStatistics();
        }
    }
}

void Effect::DebugDrawBounds()
{
    // バウンディングボックスのデバッグ描画
    // LineDrawerなどを使用して実装
}

void Effect::DebugDrawEmitters()
{
    // エミッターの位置や方向のデバッグ描画
    // LineDrawerなどを使用して実装
}

#endif // _DEBUG

} // namespace Engine
