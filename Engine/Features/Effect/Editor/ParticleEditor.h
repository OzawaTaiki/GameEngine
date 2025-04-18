#pragma once

class ParticleEditor
{
    static ParticleEditor* GetInstance();





private:

    ParticleEditor() = default;
    ~ParticleEditor() = default;
    ParticleEditor(const ParticleEditor&) = delete;
    ParticleEditor& operator=(const ParticleEditor&) = delete;

};