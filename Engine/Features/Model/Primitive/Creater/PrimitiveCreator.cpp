#include "PrimitiveCreator.h"
#include <Features/Json/JsonSerializers.h>
#include <imgui.h>
#include <filesystem>
#include <Debug/Debug.h>


namespace Engine {

PrimitiveCreator::PrimitiveCreator() :
    currentType_(PrimitiveType::Plane),
    selectedPrimitiveIndex_(-1)
{
}

PrimitiveCreator::~PrimitiveCreator()
{
}

void PrimitiveCreator::DrawImGui()
{
    if (ImGui::Begin("Primitive Creator"))
    {
        // パラメータ設定
        DrawParameterSettings();

        ImGui::Separator();

        // 作成ボタン
        if (ImGui::Button("Create Primitive", ImVec2(150, 40)))
        {
            Model* model = CreatePrimitive(currentType_, currentSettings_);
            if (model)
            {
                createdPrimitives_.emplace_back(currentType_, currentSettings_, model);
            }
        }

        ImGui::Separator();

        // 作成済みプリミティブ一覧
        DrawCreatedPrimitivesList();
    }
    ImGui::End();

    // 選択されたプリミティブのパラメータ表示
    if (selectedPrimitiveIndex_ >= 0 && selectedPrimitiveIndex_ < createdPrimitives_.size())
    {
        DrawSelectedPrimitiveParameters();
    }
}

Model* PrimitiveCreator::CreatePrimitive(PrimitiveType type, const PrimitiveSettings& settings)
{
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
    }

    if (primitive)
    {
        return primitive->Generate(settings.name);
    }

    return nullptr;
}

void PrimitiveCreator::DrawParameterSettings()
{
    // プリミティブタイプ選択
    const char* types[] = { "Plane", "Triangle", "Cylinder", "Ring","Cube"};
    int currentTypeIndex = static_cast<int>(currentType_);

    if (ImGui::Combo("Type", &currentTypeIndex, types, _countof(types)))
    {
        currentType_ = static_cast<PrimitiveType>(currentTypeIndex);
    }

    // 共通設定
    char buffer[256];
    strcpy_s(buffer, sizeof(buffer), currentSettings_.name.c_str());
    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
        currentSettings_.name = buffer;
    }


    int divide = static_cast<int>(currentSettings_.divide);
    if (ImGui::DragInt("Divide", &divide, 1, 3, 64))
    {
        currentSettings_.divide = (std::max)(3, divide);
    }

    ImGui::Checkbox("Flip U", &currentSettings_.flipU);
    ImGui::SameLine();
    ImGui::Checkbox("Flip V", &currentSettings_.flipV);

    // プリミティブ固有の設定
    switch (currentType_)
    {
    case PrimitiveType::Plane:
        DrawPlaneSettings();
        break;
    case PrimitiveType::Triangle:
        DrawTriangleSettings();
        break;
    case PrimitiveType::Cylinder:
        DrawCylinderSettings();
        break;
    case PrimitiveType::Ring:
        DrawRingSettings();
        break;
    case PrimitiveType::Cube:
        DrawCubeSettings();
        break;
    }
}

void PrimitiveCreator::DrawCreatedPrimitivesList()
{
    ImGui::Text("Created Primitives (%zu)", createdPrimitives_.size());

    if (ImGui::BeginChild("PrimitivesList", ImVec2(0, 200), true))
    {
        for (size_t i = 0; i < createdPrimitives_.size(); ++i)
        {
            const auto& primitive = createdPrimitives_[i];

            ImGui::PushID(static_cast<int>(i));

            // 選択可能なアイテム
            bool isSelected = (selectedPrimitiveIndex_ == static_cast<int>(i));
            std::string itemText = primitive.settings.name + " (" + PrimitiveTypeToString(primitive.type) + ")";

            if (primitive.isSaved)
            {
                itemText += " [Saved]";
            }

            if (ImGui::Selectable(itemText.c_str(), isSelected))
            {
                selectedPrimitiveIndex_ = static_cast<int>(i);
            }

            // 右クリックメニュー
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Save to File"))
                {
                    SavePrimitiveToFile(primitive);
                    createdPrimitives_[i].isSaved = true;
                }

                if (ImGui::MenuItem("Delete"))
                {
                    delete primitive.model;
                    createdPrimitives_.erase(createdPrimitives_.begin() + i);
                    if (selectedPrimitiveIndex_ == static_cast<int>(i))
                    {
                        selectedPrimitiveIndex_ = -1;
                    }
                    else if (selectedPrimitiveIndex_ > static_cast<int>(i))
                    {
                        selectedPrimitiveIndex_--;
                    }
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    // 一括操作
    if (ImGui::Button("Save All"))
    {
        for (auto& primitive : createdPrimitives_)
        {
            if (!primitive.isSaved)
            {
                SavePrimitiveToFile(primitive);
                primitive.isSaved = true;
            }
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear All"))
    {
        for (auto& primitive : createdPrimitives_)
        {
            delete primitive.model;
        }
        createdPrimitives_.clear();
        selectedPrimitiveIndex_ = -1;
    }
}

void PrimitiveCreator::DrawSelectedPrimitiveParameters()
{
    if (selectedPrimitiveIndex_ < 0 || selectedPrimitiveIndex_ >= createdPrimitives_.size())
        return;

    const auto& primitive = createdPrimitives_[selectedPrimitiveIndex_];
    std::string windowTitle = "Parameters: " + primitive.settings.name;

    if (ImGui::Begin(windowTitle.c_str()))
    {
        ImGui::Text("Type: %s", PrimitiveTypeToString(primitive.type));
        ImGui::Text("Name: %s", primitive.settings.name.c_str());
        ImGui::Text("Divide: %u", primitive.settings.divide);
        ImGui::Text("Flip U: %s", primitive.settings.flipU ? "Yes" : "No");
        ImGui::Text("Flip V: %s", primitive.settings.flipV ? "Yes" : "No");
        ImGui::Text("Saved: %s", primitive.isSaved ? "Yes" : "No");

        ImGui::Separator();

        // プリミティブ固有のパラメータ表示
        switch (primitive.type)
        {
        case PrimitiveType::Plane:
            ImGui::Text("Normal: (%.2f, %.2f, %.2f)",
                primitive.settings.plane.normal.x,
                primitive.settings.plane.normal.y,
                primitive.settings.plane.normal.z);
            ImGui::Text("Size: (%.2f, %.2f)",
                primitive.settings.plane.size.x,
                primitive.settings.plane.size.y);
            ImGui::Text("Pivot: (%.2f, %.2f, %.2f)",
                primitive.settings.plane.pivot.x,
                primitive.settings.plane.pivot.y,
                primitive.settings.plane.pivot.z);
            break;

        case PrimitiveType::Triangle:
            ImGui::Text("Vertex 0: (%.2f, %.2f, %.2f)",
                primitive.settings.triangle.vertex0.x,
                primitive.settings.triangle.vertex0.y,
                primitive.settings.triangle.vertex0.z);
            ImGui::Text("Vertex 1: (%.2f, %.2f, %.2f)",
                primitive.settings.triangle.vertex1.x,
                primitive.settings.triangle.vertex1.y,
                primitive.settings.triangle.vertex1.z);
            ImGui::Text("Vertex 2: (%.2f, %.2f, %.2f)",
                primitive.settings.triangle.vertex2.x,
                primitive.settings.triangle.vertex2.y,
                primitive.settings.triangle.vertex2.z);
            ImGui::Text("Normal: (%.2f, %.2f, %.2f)",
                primitive.settings.triangle.normal.x,
                primitive.settings.triangle.normal.y,
                primitive.settings.triangle.normal.z);
            break;

        case PrimitiveType::Cylinder:
            ImGui::Text("Top Radius: %.2f", primitive.settings.cylinder.topRadius);
            ImGui::Text("Bottom Radius: %.2f", primitive.settings.cylinder.bottomRadius);
            ImGui::Text("Height: %.2f", primitive.settings.cylinder.height);
            ImGui::Text("Has Top: %s", primitive.settings.cylinder.hasTop ? "Yes" : "No");
            ImGui::Text("Has Bottom: %s", primitive.settings.cylinder.hasBottom ? "Yes" : "No");
            ImGui::Text("Loop: %s", primitive.settings.cylinder.loop ? "Yes" : "No");
            ImGui::Text("Start Angle: %.2f°", primitive.settings.cylinder.startAngle * 180.0f / 3.14159f);
            ImGui::Text("End Angle: %.2f°", primitive.settings.cylinder.endAngle * 180.0f / 3.14159f);
            break;

        case PrimitiveType::Ring:
            ImGui::Text("Inner Radius: %.2f", primitive.settings.ring.innerRadius);
            ImGui::Text("Outer Radius: %.2f", primitive.settings.ring.outerRadius);
            ImGui::Text("Start Angle: %.2f°", primitive.settings.ring.startAngle * 180.0f / 3.14159f);
            ImGui::Text("End Angle: %.2f°", primitive.settings.ring.endAngle * 180.0f / 3.14159f);
            ImGui::Text("Start Outer Radius Ratio: %.2f", primitive.settings.ring.startOuterRadiusRatio);
            ImGui::Text("End Outer Radius Ratio: %.2f", primitive.settings.ring.endOuterRadiusRatio);
            break;
        case PrimitiveType::Cube:
            ImGui::Text("Size: (%.2f, %.2f, %.2f)",
                        primitive.settings.cube.size.x,
                        primitive.settings.cube.size.y,
                        primitive.settings.cube.size.z);
            ImGui::Text("Pivot: (%.2f, %.2f, %.2f)",
                        primitive.settings.cube.pivot.x,
                        primitive.settings.cube.pivot.y,
                        primitive.settings.cube.pivot.z);
            ImGui::Text("Has Top: %s", primitive.settings.cube.hasTop ? "Yes" : "No");
            ImGui::Text("Has Bottom: %s", primitive.settings.cube.hasBottom ? "Yes" : "No");
            break;
        }


        ImGui::Separator();

        if (!primitive.isSaved)
        {
            if (ImGui::Button("Save to File"))
            {
                SavePrimitiveToFile(primitive);
                createdPrimitives_[selectedPrimitiveIndex_].isSaved = true;
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Already Saved");
        }
    }
    ImGui::End();
}

void PrimitiveCreator::SavePrimitiveToFile(const CreatedPrimitive& primitive)
{
    try
    {
        // JsonSerializerを使用してシンプルに保存
        json j;
        j["type"] = primitive.type;           // to_json(json&, const PrimitiveType&) が呼ばれる
        j["settings"] = primitive.settings;   // to_json(json&, const PrimitiveSettings&) が呼ばれる
        j["version"] = "1.0";
        j["createdTime"] = std::time(nullptr);

        // ディレクトリを作成
        std::filesystem::create_directories("Resources/Primitives/");

        // ファイルに保存
        JsonFileIO::Save(primitive.settings.name, "Resources/Primitives/", j);

        Debug::Log("Primitive saved using JsonSerializer: " + primitive.settings.name + "\n");
    }
    catch (const std::exception& e)
    {
        Debug::Log("Failed to save primitive: " + std::string(e.what()) + "\n");
    }
}

void PrimitiveCreator::DrawPlaneSettings()
{
    ImGui::DragFloat3("Normal", &currentSettings_.plane.normal.x, 0.01f, -1.0f, 1.0f);
    ImGui::DragFloat2("Size", &currentSettings_.plane.size.x, 0.1f, 0.1f, 10.0f);
    ImGui::DragFloat3("Pivot", &currentSettings_.plane.pivot.x, 0.01f, -1.0f, 1.0f);
}

void PrimitiveCreator::DrawTriangleSettings()
{
    ImGui::DragFloat3("Vertex 0", &currentSettings_.triangle.vertex0.x, 0.01f);
    ImGui::DragFloat3("Vertex 1", &currentSettings_.triangle.vertex1.x, 0.01f);
    ImGui::DragFloat3("Vertex 2", &currentSettings_.triangle.vertex2.x, 0.01f);
    ImGui::DragFloat3("Normal", &currentSettings_.triangle.normal.x, 0.01f, -1.0f, 1.0f);
}

void PrimitiveCreator::DrawCylinderSettings()
{
    ImGui::DragFloat("Top Radius", &currentSettings_.cylinder.topRadius, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Bottom Radius", &currentSettings_.cylinder.bottomRadius, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Height", &currentSettings_.cylinder.height, 0.01f, 0.1f, 10.0f);

    ImGui::Checkbox("Has Top", &currentSettings_.cylinder.hasTop);
    ImGui::SameLine();
    ImGui::Checkbox("Has Bottom", &currentSettings_.cylinder.hasBottom);
    ImGui::Checkbox("Loop", &currentSettings_.cylinder.loop);

    float startAngleDeg = currentSettings_.cylinder.startAngle * 180.0f / 3.14159f;
    float endAngleDeg = currentSettings_.cylinder.endAngle * 180.0f / 3.14159f;

    if (ImGui::DragFloat("Start Angle (deg)", &startAngleDeg, 1.0f, 0.0f, 360.0f))
    {
        currentSettings_.cylinder.startAngle = startAngleDeg * 3.14159f / 180.0f;
    }

    if (ImGui::DragFloat("End Angle (deg)", &endAngleDeg, 1.0f, 0.0f, 360.0f))
    {
        currentSettings_.cylinder.endAngle = endAngleDeg * 3.14159f / 180.0f;
    }
}

void PrimitiveCreator::DrawRingSettings()
{
    ImGui::DragFloat("Inner Radius", &currentSettings_.ring.innerRadius, 0.01f, 0.0f, 10.0f);
    ImGui::DragFloat("Outer Radius", &currentSettings_.ring.outerRadius, 0.01f, 0.0f, 10.0f);

    float startAngleDeg = currentSettings_.ring.startAngle * 180.0f / 3.14159f;
    float endAngleDeg = currentSettings_.ring.endAngle * 180.0f / 3.14159f;

    if (ImGui::DragFloat("Start Angle (deg)", &startAngleDeg, 1.0f, 0.0f, 360.0f))
    {
        currentSettings_.ring.startAngle = startAngleDeg * 3.14159f / 180.0f;
    }

    if (ImGui::DragFloat("End Angle (deg)", &endAngleDeg, 1.0f, 0.0f, 360.0f))
    {
        currentSettings_.ring.endAngle = endAngleDeg * 3.14159f / 180.0f;
    }

    ImGui::DragFloat("Start Outer Radius Ratio", &currentSettings_.ring.startOuterRadiusRatio, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat("End Outer Radius Ratio", &currentSettings_.ring.endOuterRadiusRatio, 0.01f, 0.0f, 1.0f);
}

void PrimitiveCreator::DrawCubeSettings()
{
    ImGui::DragFloat3("Size", &currentSettings_.cube.size.x, 0.1f, 0.1f, 10.0f);
    ImGui::DragFloat3("Pivot", &currentSettings_.cube.pivot.x, 0.01f, -1.0f, 1.0f);
    ImGui::Checkbox("Has Top", &currentSettings_.cube.hasTop);
    ImGui::SameLine();
    ImGui::Checkbox("Has Bottom", &currentSettings_.cube.hasBottom);
}

const char* PrimitiveCreator::PrimitiveTypeToString(PrimitiveType type)
{
    switch (type)
    {
    case PrimitiveType::Plane: return "Plane";
    case PrimitiveType::Triangle: return "Triangle";
    case PrimitiveType::Cylinder: return "Cylinder";
    case PrimitiveType::Ring: return "Ring";
    case PrimitiveType::Cube: return "Cube";
    default: return "Unknown";
    }
}

} // namespace Engine
