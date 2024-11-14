#include "Skeleton.h"
#include "../Node/Node.h"

void Skeleton::Initialize()
{
}

void Skeleton::Update()
{
    for (Joint& joint : joints_)
    {
        joint.Update(joints_);
    }
}

void Skeleton::Draw(const Matrix4x4& _wMat)
{
    for (Joint& joint : joints_)
    {
        joint.Draw(_wMat, joints_);
    }
}

void Skeleton::CreateSkeleton(const Node& _node)
{
    rootIndex_ = Joint::CreateJoint(_node, {}, joints_);
    for (const Joint& joint : joints_)
    {
        jointMap_.emplace(joint.name_, joint.index_);
    }
}
