#include "RotationBasedMovementModifier.h"

#include <Math/Vector/VectorFunction.h>

void RotationBasedMovementModifier::Apply(Particle* particle, [[maybe_unused]] float deltaTime)
{

    if (!particle) return;

    Vector3 rotation = particle->GetRotation();

    Vector3 upVector = Vector3(0, 1, 0);// 上方向

    Vector3 rotVector = Transform(upVector, MakeRotateMatrix(rotation));

    particle->SetDirection(rotVector.Normalize());

}
