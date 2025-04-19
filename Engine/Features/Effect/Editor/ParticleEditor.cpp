#include "ParticleEditor.h"

ParticleEditor* ParticleEditor::GetInstance()
{
    static ParticleEditor instance;
    return &instance;
}
