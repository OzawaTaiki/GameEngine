#include <Features/Json/Loader/JsonFileIO.h>
#include <Utility/StringUtils/StringUitls.h>
#include <Debug/Debug.h>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>


JsonFileIO::JsonFileIO()
{
}

JsonFileIO::~JsonFileIO() {
}


json JsonFileIO::Load(const std::string& _filepath, const std::string& _directory)
{
    std::string filepath = _directory + _filepath;
    if (StringUtils::GetExtension(filepath).empty())
        filepath += ".json";

    Debug::Log("JsonFileIO::Load filepath: " + filepath + "\n");

    std::ifstream inputFile(filepath);
    if (!inputFile.is_open())
    {
        Debug::Log("Cant Open inputFile\n");
        return json();
    }

    json j;
    if (inputFile.peek() != std::ifstream::traits_type::eof())
    {
        inputFile >> j;
    }
    inputFile.close();

    Debug::Log("Load Success \n");

    return j;
}

void JsonFileIO::Save(const std::string& _filepath, const std::string& _directory, const json& _data)
{
    std::string filepath = _directory + _filepath;

    if (StringUtils::GetExtension(filepath).empty())
        filepath += ".json";

    Debug::Log("JsonFileIO::Save filepath: " + filepath + "\n");

    std::string dir = _directory;
    if(dir.empty())
        dir = StringUtils::GetBeforeLast(filepath, "/");

    std::filesystem::create_directories(dir);

    std::ofstream outputFile(filepath);
    if (!outputFile.is_open())
    {
        assert(outputFile.is_open() && "Cant Open outputFile");
        return;
    }


    outputFile << _data.dump(4); // 4スペースでインデント

    outputFile.close();

    Debug::Log("Save Success \n");
}
