#include <Features/Json/JsonBinder.h>


namespace Engine {

JsonBinder::JsonBinder(const std::string& _name, const std::string& _directioy)
{
    groupName_ = _name;

    if (_directioy.back() != '/')
        folderPath_ = _directioy + "/";
    else
        folderPath_ = _directioy;

    jsonData_ = JsonFileIO::Load(groupName_ + ".json", folderPath_);

}
void JsonBinder::Save()
{
    for (auto& [key, value] : memberPtrMap_)
    {
        jsonData_[groupName_][key] = value.Save();
    }

    JsonFileIO::Save(groupName_ + ".json", folderPath_, jsonData_);
}

} // namespace Engine
