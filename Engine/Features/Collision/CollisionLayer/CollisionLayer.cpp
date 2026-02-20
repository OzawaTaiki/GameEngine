#include "CollisionLayer.h"

#include <Features/Collision/CollisionLayer/CollisionLayerManager.h>
#include <Features/Json/JsonBinder.h>


namespace Engine {

CollisionLayer::CollisionLayer()
    : layer_(0)
    , layerMask_(0)
{
}

void CollisionLayer::SetLayer(const std::string& _layer)
{
    layer_ |= CollisionLayerManager::GetInstance()->GetLayer(_layer);
}

void CollisionLayer::SetLayerMask(const std::string& _layer)
{
    layerMask_ |= CollisionLayerManager::GetInstance()->GetLayer(_layer);
}

void CollisionLayer::SetCollisionLayer(const std::string& _layer)
{
    uint32_t layer = ~CollisionLayerManager::GetInstance()->GetLayer(_layer);
    layerMask_ = layer;
}

void CollisionLayer::AddCollisionLayer(const std::string& _layer)
{
    uint32_t layer = ~CollisionLayerManager::GetInstance()->GetLayer(_layer);
    uint32_t xnorMask = ~(layerMask_ ^ layer);


    layerMask_ = xnorMask;
}

void CollisionLayer::ExcludeLayer(const std::string& _layer)
{
    layer_ &= ~CollisionLayerManager::GetInstance()->GetLayer(_layer);
}

void CollisionLayer::ExcludeLayerMask(const std::string& _layer)
{
    layerMask_ &= ~CollisionLayerManager::GetInstance()->GetLayer(_layer);
}

void CollisionLayer::RegisterLayer(JsonBinder* _jsonBinder)
{
    _jsonBinder->RegisterVariable("layer", &layer_);
    _jsonBinder->RegisterVariable("layerMask", &layerMask_);
}

} // namespace Engine
