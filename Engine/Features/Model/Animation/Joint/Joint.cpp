#include <Features/Model/Animation/Joint/Joint.h>

#include <Features/Model/Animation/Node/Node.h>
#include <Math/Matrix/MatrixFunction.h>
#include <Features/LineDrawer/LineDrawer.h>
#include <Math/Vector/VectorFunction.h>

void Joint::Initialize()
{
}

void Joint::Update(std::vector<Joint>& _joints)
{
    localMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translate);
    if (parentIndex_)
    {
        SkeletonSpcaceMatrix_ = localMatrix_ * _joints[*parentIndex_].SkeletonSpcaceMatrix_;
    }
    else
    {
        SkeletonSpcaceMatrix_ = localMatrix_;
    }
}


void Joint::Draw(const Matrix4x4& _wMat, std::vector <Joint>& _joints)
{
    Matrix4x4 wMat = SkeletonSpcaceMatrix_ * _wMat;
    static Matrix4x4 sMat = MakeScaleMatrix({ 0.1f,0.1f ,0.1f });

    Vector4 color = openTree_ ? Vector4{ 1,0,0,1 } : Vector4{ 1,1,1,1 };

    LineDrawer::GetInstance()->DrawOBB(sMat * wMat, color, true);
    Vector3 pos = Transform({ 0,0,0 }, wMat);
    for (int32_t childIndex : children_)
    {
        Vector3 childPos = Transform({ 0,0,0 }, _joints[childIndex].SkeletonSpcaceMatrix_ * _wMat);
        LineDrawer::GetInstance()->RegisterPoint(pos, childPos, color, true);
    }
}

int32_t Joint::CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints)
{
    Joint joint = {};
    joint.name_ = _node.name_;
    joint.localMatrix_ = _node.GetLocalMatrix();
    joint.SkeletonSpcaceMatrix_ = MakeIdentity4x4();
    joint.transform_ = _node.transform_;
    joint.idleTransform_ = joint.transform_;
    joint.index_ = static_cast<int32_t>(_joints.size());
    joint.parentIndex_ = _parent;

    _joints.push_back(joint);
    for (const Node& child : _node.children_)
    {
        int32_t childIndex = CreateJoint(child, joint.index_, _joints);
        _joints[joint.index_].children_.push_back(childIndex);
    }


    return joint.index_;
}

void Joint::ImGui(std::unordered_map<std::string, int32_t>& _map, int32_t indent)
{
#ifdef _DEBUG

    if (_map.contains(name_))
        return;

    _map[name_] = index_;

    ImGui::PushID(this);
    std::string label = std::string(indent, '\t') + name_;

    openTree_ = false;

    if (ImGui::TreeNode(label.c_str()))
    {
        ImGui::Text("Index : %d", index_);
        ImGui::Text("Children Count : %d", static_cast<int32_t>(children_.size()));
        ImGui::Text("Transform : ");
        ImGui::Text("  Scale : %.2f, %.2f, %.2f", transform_.scale.x, transform_.scale.y, transform_.scale.z);
        ImGui::Text("  Rotation : %.2f, %.2f, %.2f", transform_.rotation.x, transform_.rotation.y, transform_.rotation.z);
        ImGui::Text("  Translate : %.2f, %.2f, %.2f", transform_.translate.x, transform_.translate.y, transform_.translate.z);

        openTree_ = true;

        ImGui::TreePop();
    }

    ImGui::PopID();

    indent += 2;

#endif // _DEBUG
}
