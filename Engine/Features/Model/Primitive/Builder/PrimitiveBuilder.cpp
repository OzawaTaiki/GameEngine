#include "PrimitiveBuilder.h"
#include <Features/Json/JsonSerializers.h>
#include <Features/Model/Primitive/Creater/PrimitiveCreator.h>
#include <Features/Model/Primitive/Plane.h>
#include <Features/Model/Primitive/Triangle.h>
#include <Features/Model/Primitive/Cylinder.h>
#include <Features/Model/Primitive/Ring.h>
#include <Debug/Debug.h>
#include <filesystem>


namespace Engine {

void PrimitiveBuilder::BuildAndRegisterAll(const std::string& directory)
{
    if (!std::filesystem::exists(directory))
    {
        Debug::Log("Primitives directory not found: " + directory + "\n");
        return;
    }

    int builtCount = 0;

    try
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                std::string fileName = entry.path().stem().string();

                Model* model = BuildFromFile(fileName, directory);
                if (model)
                {
                    builtCount++;
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        Debug::Log("Error during primitive build: " + std::string(e.what()) + "\n");
    }

    Debug::Log("PrimitiveBuilder: Built " + std::to_string(builtCount) + " primitives\n");
}

Model* PrimitiveBuilder::BuildFromFile(const std::string& fileName, const std::string& directory)
{
    try
    {
        json j = JsonFileIO::Load(fileName, directory);

        if (j.empty())
        {
            Debug::Log("Failed to load primitive file: " + fileName + "\n");
            return nullptr;
        }

        return CreateModelFromJson(j);
    }
    catch (const std::exception& e)
    {
        Debug::Log("Error building primitive from file " + fileName + ": " + e.what() + "\n");
        return nullptr;
    }
}

Model* PrimitiveBuilder::CreateModelFromJson(const json& j)
{
    try
    {
        // JsonSerializerを使用してシンプルに読み込み
        PrimitiveType type = j["type"].get<PrimitiveType>();           // from_json() が呼ばれる
        PrimitiveSettings settings = j["settings"].get<PrimitiveSettings>(); // from_json() が呼ばれる

        std::unique_ptr<Primitive> primitive;

        switch (type)
        {
        case PrimitiveType::Plane:
        {
            auto plane = std::make_unique<Plane>();
            plane->SetNormal(settings.plane.normal);
            plane->SetSize(settings.plane.size);
            plane->SetPivot(settings.plane.pivot);
            plane->SetFlipU(settings.flipU);
            plane->SetFlipV(settings.flipV);
            primitive = std::move(plane);
            break;
        }
        case PrimitiveType::Triangle:
        {
            auto triangle = std::make_unique<Triangle>();
            triangle->SetVertices(settings.triangle.vertex0, settings.triangle.vertex1, settings.triangle.vertex2);
            triangle->SetNormal(settings.triangle.normal);
            triangle->SetFlipU(settings.flipU);
            triangle->SetFlipV(settings.flipV);
            primitive = std::move(triangle);
            break;
        }
        case PrimitiveType::Cylinder:
        {
            auto cylinder = std::make_unique<Cylinder>(settings.cylinder.topRadius, settings.cylinder.bottomRadius, settings.cylinder.height);
            cylinder->SetDivide(settings.divide);
            cylinder->SetTop(settings.cylinder.hasTop);
            cylinder->SetBottom(settings.cylinder.hasBottom);
            cylinder->SetStartAngle(settings.cylinder.startAngle);
            cylinder->SetEndAngle(settings.cylinder.endAngle);
            cylinder->SetLoop(settings.cylinder.loop);
            cylinder->SetFlipU(settings.flipU);
            cylinder->SetFlipV(settings.flipV);
            primitive = std::move(cylinder);
            break;
        }
        case PrimitiveType::Ring:
        {
            auto ring = std::make_unique<Ring>(settings.ring.innerRadius, settings.ring.outerRadius);
            ring->SetDivide(settings.divide);
            ring->SetStartAngle(settings.ring.startAngle);
            ring->SetEndAngle(settings.ring.endAngle);
            ring->SetStartOuterRadiusRatio(settings.ring.startOuterRadiusRatio);
            ring->SetEndOuterRadiusRatio(settings.ring.endOuterRadiusRatio);
            ring->SetFlipU(settings.flipU);
            ring->SetFlipV(settings.flipV);
            primitive = std::move(ring);
            break;
        }
        case PrimitiveType::Cube:
        {
            auto cube = std::make_unique<Cube>();
            cube->SetSize(settings.cube.size);
            cube->SetPivot(settings.cube.pivot);
            cube->HasTop(settings.cube.hasTop);
            cube->HasBottom(settings.cube.hasBottom);
            cube->SetFlipU(settings.flipU);
            cube->SetFlipV(settings.flipV);
            primitive = std::move(cube);
            break;
        }
        default:
            Debug::Log("Unknown primitive type\n");
            return nullptr;
        }

        if (primitive)
        {
            return primitive->Generate(settings.name);
        }

        return nullptr;
    }
    catch (const std::exception& e)
    {
        Debug::Log("Error creating model from JSON: " + std::string(e.what()) + "\n");
        return nullptr;
    }
}

} // namespace Engine
