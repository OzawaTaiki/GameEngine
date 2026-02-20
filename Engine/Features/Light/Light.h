#pragma once

#include <Math/Vector/Vector3.h>
#include <Math/Vector/Vector4.h>
#include <Math/Matrix/Matrix4x4.h>

#include <string>



namespace Engine {

class Light
{
public:

    virtual ~Light() = default;

    virtual void Update() = 0;

    virtual bool IsCastShadow() const = 0;

    const std::string& GetName() const { return name_; }
    void SetName(const std::string& _name) { name_ = _name; }

    void SetParent(Vector3* _parent) { parent_ = _parent; }
    Vector3* GetParent() const { return parent_; }

protected:

    std::string name_;
    Vector3* parent_ = nullptr;


};

} // namespace Engine
