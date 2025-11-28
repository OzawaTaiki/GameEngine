#include <Features/Sprite/Sprite.h>

#include <Core/DXCommon/TextureManager/TextureManager.h>
#include <Features/Sprite/SpriteManager.h>
#include <Core/DXCommon/DXCommon.h>
#include <Core/WinApp/WinApp.h>
#include <Math/Matrix/MatrixFunction.h>

#include <Debug/ImGuiDebugManager.h>

uint32_t Sprite::winWidth_ = WinApp::kWindowWidth_;
uint32_t Sprite::winHeight_ = WinApp::kWindowHeight_;

Sprite::Sprite(const std::string& _name, bool _debugWinddow)
{
    if (_debugWinddow)
        name_ = ImGuiDebugManager::GetInstance()->AddDebugWindow(_name, [&]() { ImGui(); });
}

Sprite::~Sprite()
{
    //ImGuiDebugManager::GetInstance()->RemoveDebugWindow(name_);
}

void Sprite::Initialize()
{
    if (textureHandle_ == UINT32_MAX)
        SetTextureHandle(TextureManager::GetInstance()->Load("white.png"));

    defaultTextureSize_ = TextureManager::GetInstance()->GetTextureSize(textureHandle_);

    vertexData_.resize(6);

    isDirty_ = true;
    isVertexDirty_ = true;

    UpdateVertexData();
    UpdateInstanceData();
}

void Sprite::Initialize(uint32_t _textureHandle)
{
    SetTextureHandle(_textureHandle);
    Initialize();
}

void Sprite::Update()
{
    UpdateVertexData();
    UpdateInstanceData();

#ifdef _DEBUG
    ImGui();
#endif // _DEBUG
}

void Sprite::Draw()
{
    UpdateVertexData();
    UpdateInstanceData();
    Batch2DRenderer::GetInstance()->AddInstace(instanceData_, vertexData_, order_);
}

void Sprite::Draw(const Vector4& _color)
{
    SetColor(_color);
    Draw();
}


std::unique_ptr<Sprite> Sprite::Create(const std::string& _name, uint32_t _textureHandle, const Vector2& _anchor)
{
    std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>(_name);
    sprite->textureHandle_ = _textureHandle;
    sprite->anchor_ = _anchor;
    sprite->Initialize();

    return std::move(sprite);
}

void Sprite::StaticInitialize(uint32_t _windowWidth, uint32_t _windowWHeight)
{
    SpriteManager::GetInstance()->Initialize();
    winWidth_ = _windowWidth;
    winHeight_ = _windowWHeight;
}

void Sprite::PreDraw()
{
    SpriteManager::GetInstance()->PreDraw();
}

void Sprite::SetTextureHandle(uint32_t _textureHandle)
{
    textureHandle_ = _textureHandle;
    isDirty_ = true;
}

void Sprite::SetAnchor(const Vector2& _anchor)
{
    anchor_ = _anchor;
    isVertexDirty_ = true;
}
void Sprite::SetSize(const Vector2& _size)
{
    scale_ = _size / defaultTextureSize_;
    isDirty_ = true;
}

void Sprite::SetColor(const Vector4& _color)
{
    color_ = _color;
    isDirty_ = true;
}

void Sprite::SetUVSize(const Vector2& _size)
{
    uvScale_ = _size / defaultTextureSize_;
    isDirty_ = true;
}

void Sprite::SetLeftTop(const Vector2& _leftTop)
{
    uvTranslate_ = _leftTop / defaultTextureSize_;
    isDirty_ = true;
}

bool Sprite::IsHitTest(const Vector2& _mousePos) const
{
    // スプライトの四隅のワールド座標を計算
    Vector2 size = defaultTextureSize_ * scale_;
    Vector2 topLeft = translate_ - Vector2(anchor_.x * size.x, (1.0f - anchor_.y) * size.y);
    Vector2 bottomRight = topLeft + size;

    // マウスポインタがスプライトの矩形内にあるか判定
    return (_mousePos.x >= topLeft.x && _mousePos.x <= bottomRight.x &&
            _mousePos.y >= topLeft.y && _mousePos.y <= bottomRight.y);
}

void Sprite::UpdateVertexData()
{
    if (!isVertexDirty_)
        return;

    Vector2 size = defaultTextureSize_;

    vertexData_[0] = {
        .position ={-anchor_.x * size.x ,(1.0f - anchor_.y) * size.y ,0.0f,1.0f },
        .texCoord = { 0.0f,1.0f },
        .color = Vector4(1.0f,1.0f,1.0f,1.0f)
    };
    vertexData_[1] = {
        .position ={-anchor_.x * size.x ,-anchor_.y * size.y ,0.0f,1.0f },
        .texCoord = { 0.0f,0.0f },
        .color = Vector4(1.0f,1.0f,1.0f,1.0f)
    };
    vertexData_[2] = {
        .position ={(1.0f - anchor_.x) * size.x ,(1.0f - anchor_.y) * size.y ,0.0f,1.0f },
        .texCoord = { 1.0f,1.0f },
        .color = Vector4(1.0f,1.0f,1.0f,1.0f)
    };
    vertexData_[3] = vertexData_[1];
    vertexData_[4] = {
        .position ={(1.0f - anchor_.x) * size.x ,-anchor_.y * size.y ,0.0f,1.0f },
        .texCoord = { 1.0f,0.0f },
        .color = Vector4(1.0f,1.0f,1.0f,1.0f)
    };
    vertexData_[5] = vertexData_[2];

    isVertexDirty_ = false;
}

void Sprite::UpdateInstanceData()
{
    if (!isDirty_)
        return;

    Vector3 s = { scale_,1.0f };
    Vector3 r = { 0.0f,0.0f ,rotate_ };
    Vector3 t = { translate_,0.0f };
    instanceData_.transform = MakeAffineMatrix(s, r, t);

    s = { uvScale_,1.0f };
    r = { 0.0f,0.0f ,uvRotate_ };
    t = { uvTranslate_,0.0f };
    instanceData_.uvTransform = MakeAffineMatrix(s, r, t);

    instanceData_.color = color_;
    instanceData_.textureIndex = TextureManager::GetInstance()->GetSRVIndex(textureHandle_);
    instanceData_.useTextureAlpha = 0;

    isDirty_ = false;
}
void Sprite::ImGui()
{
#ifdef _DEBUG
    bool isDirty = false;
    ImGui::PushID(this);
    isDirty |= ImGui::DragFloat2("Translate", &translate_.x);
    isDirty |= ImGui::DragFloat2("Scale", &scale_.x, 0.01f);
    isDirty |= ImGui::DragFloat("Rotate", &rotate_, 0.01f);
    isDirty |= ImGui::DragFloat2("UVTranslate", &uvTranslate_.x, 0.01f);
    isDirty |= ImGui::DragFloat2("UVScale", &uvScale_.x, 0.01f);
    isDirty |= ImGui::DragFloat("UVRotate", &uvRotate_, 0.01f);
    isDirty |= ImGui::ColorEdit4("Color", &color_.x);
    isDirty |= ImGui::DragFloat2("Anchor", &anchor_.x, 0.01f);
    isDirty |= ImGui::DragFloat2("Size", &size_.x, 0.01f);
    isDirty |= ImGui::DragFloat2("LeftTop", &lefttop_.x, 0.01f);
    int order = order_;
    ImGui::InputInt("Order", &order);
    order_ = static_cast<int16_t>(order);
    if (ImGui::Button("Set"))
    {
        SetUVSize(size_);
        SetLeftTop(lefttop_);
    }

    if (isDirty)
    {
        isDirty_ = true;
        isVertexDirty_ = true;
        UpdateVertexData();
        UpdateInstanceData();
    }

    ImGui::PopID();
#endif // _DEBUG
}
