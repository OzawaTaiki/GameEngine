#include "Collider.h"
#include "CollisionManager.h"

void Collider::SetAtrribute(const std::string& _atrribute)
{
    atrribute_ = CollisionManager::GetInstance()->GetAtttibute(_atrribute);
}

void Collider::SetMask(const std::string& _atrribute)
{
    mask_ = CollisionManager::GetInstance()->GetMask(_atrribute);
}

void Collider::SetMask(std::initializer_list<std::string> _atrribute)
{
    mask_ = 0xffffffffu;
    for (std::string str : _atrribute)
    {
        mask_ &= CollisionManager::GetInstance()->GetMask(str);
    }
}
