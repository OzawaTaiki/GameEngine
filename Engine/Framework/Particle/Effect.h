#pragma once

#include <Framework/Particle/ParticleEmitters.h>
#include <Systems/JsonBinder/JsonBinder.h>
#include <string>
#include <vector>
#include <list>
#include <memory>

class Effect
{
private:

public:
    Effect() = default;
    // TODO :emitterの解放周りでエラーが出る
    // 所有権がどこにあるか明確にする
    ~Effect() = default;

    void Initialize(const std::string& _name);
    void Update();

    ParticleEmitter* AddEmitter(const std::string& _name);

    std::list<ParticleEmitter*> GetEmitters() const;
    std::string GetName() const { return name_; }

    void SetActive(bool _active);

    void ExclusionEmitter(const std::string& _name);

    void Save()const;
private:
    void Reset();

    static const uint32_t          kMaxEmitters = 20;  // 最大エミッター数

    std::string                     name_;              // エフェクトの名前
    std::list<ParticleEmitter>      emitters_;          // エミッターのリスト
    std::vector<std::string>        emitterNames_;      // エミッターの名前リスト


    float                           elapsedTime_;       // 経過時間
    bool                            isLoop_;            // ループするか
    bool                            isActive_;          // アクティブか

    std::unique_ptr<JsonBinder>     jsonBinder_ = nullptr;  // 設定ファイル
    //std::unique_ptr<Config> config_ = nullptr;
};
