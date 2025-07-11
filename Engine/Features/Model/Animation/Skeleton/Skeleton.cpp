#include <Features/Model/Animation/Skeleton/Skeleton.h>
#include <Features/Model/Animation/Node/Node.h>

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

const Matrix4x4* Skeleton::GetSkeletonSpaceMatrix(const std::string& _name) const
{
    auto it = jointMap_.find(_name);
    if (it != jointMap_.end())
    {
        return joints_[it->second].GetSkeletonSpaceMatrix();
    }

    return nullptr;
}

#ifdef _DEBUG
void Skeleton::ImGui()
{

    ImGui::SeparatorText("Skeleton");
    ImGui::Text("Root Joint : %s", joints_[rootIndex_].name_.c_str());
    ImGui::Text("Joint Count : %d", static_cast<int32_t>(joints_.size()));

    debugJointMap_.clear();

    Joint& joint = joints_[rootIndex_];

    Show(joint, 0);

}

void Skeleton::Show(Joint& _joint, uint32_t _indent)
{

    _joint.ImGui(debugJointMap_, _indent);

    auto children = _joint.GetChildren();
    for (int32_t childIndex : children)
    {
        if (debugJointMap_.contains(joints_[childIndex].name_))
            continue;
        Show(joints_[childIndex], _indent + 1);
    }

}
#endif // _DEBUG
