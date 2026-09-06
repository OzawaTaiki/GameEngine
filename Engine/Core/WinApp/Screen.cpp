#include <Core/WinApp/Screen.h>


namespace Engine {

namespace Screen {

namespace {

// ここが唯一の実体。3つ持つと片方だけ更新されて壊れるのでサイズは常にここから作る
uint32_t width_ = 1280;
uint32_t height_ = 720;
Vector2 size_ = { 1280.0f, 720.0f };

} // namespace

void SetSize(uint32_t _width, uint32_t _height)
{
    // 0除算を避けるため最低1ピクセルは確保する
    width_ = _width > 0 ? _width : 1;
    height_ = _height > 0 ? _height : 1;

    size_ = Vector2{ static_cast<float>(width_), static_cast<float>(height_) };
}

uint32_t Width()
{
    return width_;
}

uint32_t Height()
{
    return height_;
}

float WidthF()
{
    return size_.x;
}

float HeightF()
{
    return size_.y;
}

const Vector2& Size()
{
    return size_;
}

float Aspect()
{
    return size_.x / size_.y;
}

Vector2 Center()
{
    return Vector2{ size_.x * 0.5f, size_.y * 0.5f };
}

Vector2 ToNormalized(const Vector2& _position)
{
    return Vector2{ _position.x / size_.x, _position.y / size_.y };
}

Vector2 FromNormalized(const Vector2& _normalized)
{
    return Vector2{ _normalized.x * size_.x, _normalized.y * size_.y };
}

} // namespace Screen


namespace Window {

namespace {

uint32_t width_ = 1280;
uint32_t height_ = 720;
Vector2 size_ = { 1280.0f, 720.0f };

} // namespace

void SetSize(uint32_t _width, uint32_t _height)
{
    width_ = _width > 0 ? _width : 1;
    height_ = _height > 0 ? _height : 1;

    size_ = Vector2{ static_cast<float>(width_), static_cast<float>(height_) };
}

uint32_t Width()
{
    return width_;
}

uint32_t Height()
{
    return height_;
}

const Vector2& Size()
{
    return size_;
}

float Aspect()
{
    return size_.x / size_.y;
}

} // namespace Window

} // namespace Engine
