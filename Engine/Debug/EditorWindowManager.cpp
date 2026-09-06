#include <Debug/EditorWindowManager.h>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG

#include <algorithm>


namespace Engine {

//==============================================================================
// EditorWindowHandle
//==============================================================================

EditorWindowHandle::~EditorWindowHandle()
{
    Release();
}

EditorWindowHandle::EditorWindowHandle(EditorWindowHandle&& _other) noexcept :
    id_(_other.id_),
    name_(std::move(_other.name_))
{
    _other.id_ = 0;
}

EditorWindowHandle& EditorWindowHandle::operator=(EditorWindowHandle&& _other) noexcept
{
    if (this == &_other)
        return *this;

    Release();

    id_ = _other.id_;
    name_ = std::move(_other.name_);
    _other.id_ = 0;

    return *this;
}

void EditorWindowHandle::Release()
{
    if (id_ == 0)
        return;

    EditorWindowManager::GetInstance()->Unregister(id_);
    id_ = 0;
    name_.clear();
}

void EditorWindowHandle::SetVisible(bool _visible)
{
    if (id_ == 0)
        return;

    EditorWindowManager::GetInstance()->SetVisible(id_, _visible);
}

bool EditorWindowHandle::IsVisible() const
{
    if (id_ == 0)
        return false;

    return EditorWindowManager::GetInstance()->IsVisible(id_);
}

void EditorWindowHandle::Focus()
{
    if (id_ == 0)
        return;

    EditorWindowManager* manager = EditorWindowManager::GetInstance();
    manager->SetVisible(id_, true);
    manager->RequestFocus(id_);
}


//==============================================================================
// EditorWindowManager
//==============================================================================

EditorWindowManager* EditorWindowManager::GetInstance()
{
    static EditorWindowManager instance;
    return &instance;
}

std::string EditorWindowManager::MakeUniqueName(const std::string& _name) const
{
    const auto isUsed = [this](const std::string& _candidate)
        {
            return std::any_of(windows_.begin(), windows_.end(),
                               [&_candidate](const auto& _pair) { return _pair.second.name == _candidate; });
        };

    if (!isUsed(_name))
        return _name;

    for (int32_t i = 0; i < 10000; ++i)
    {
        std::string candidate = _name + std::to_string(i);
        if (!isUsed(candidate))
            return candidate;
    }

    return _name;
}

uint64_t EditorWindowManager::RegisterInternal(const std::string& _name,
                                               std::function<void()> _drawFunc,
                                               const std::string& _category,
                                               EditorWindowFlags _flags,
                                               int32_t _windowFlags,
                                               std::string& _outName)
{
    if (!_drawFunc)
    {
        _outName.clear();
        return 0;
    }

    WindowEntry entry;
    entry.name = MakeUniqueName(_name);
    entry.category = _category.empty() ? std::string("Custom") : _category;
    entry.drawFunc = std::move(_drawFunc);
    entry.flags = _flags;
    entry.windowFlags = _windowFlags;
    entry.isVisible = HasFlag(_flags, EditorWindowFlags::DefaultOpen);

    const uint64_t id = nextID_++;
    _outName = entry.name;
    windows_.emplace(id, std::move(entry));

    return id;
}

EditorWindowHandle EditorWindowManager::Register(const std::string& _name,
                                                 std::function<void()> _drawFunc,
                                                 const std::string& _category,
                                                 EditorWindowFlags _flags,
                                                 int32_t _windowFlags)
{
    std::string registeredName;
    const uint64_t id = RegisterInternal(_name, std::move(_drawFunc), _category, _flags, _windowFlags, registeredName);

    if (id == 0)
        return EditorWindowHandle();

    return EditorWindowHandle(id, registeredName);
}

std::string EditorWindowManager::RegisterPersistent(const std::string& _name,
                                                    std::function<void()> _drawFunc,
                                                    const std::string& _category,
                                                    EditorWindowFlags _flags,
                                                    int32_t _windowFlags)
{
    std::string registeredName;
    RegisterInternal(_name, std::move(_drawFunc), _category, _flags, _windowFlags, registeredName);

    return registeredName;
}

void EditorWindowManager::Unregister(uint64_t _id)
{
    auto it = windows_.find(_id);
    if (it == windows_.end())
        return;

    // 描画中はコールバックの実行中かもしれないので消さずに予約する
    if (isDrawing_)
    {
        it->second.isVisible = false;
        pendingRemove_.push_back(_id);
        return;
    }

    windows_.erase(it);
}

void EditorWindowManager::SetVisible(uint64_t _id, bool _visible)
{
    auto it = windows_.find(_id);
    if (it == windows_.end())
        return;

    it->second.isVisible = _visible;
}

void EditorWindowManager::RequestFocus(uint64_t _id)
{
    auto it = windows_.find(_id);
    if (it == windows_.end())
        return;

    it->second.requestFocus = true;
}

bool EditorWindowManager::IsVisible(uint64_t _id) const
{
    auto it = windows_.find(_id);
    if (it == windows_.end())
        return false;

    return it->second.isVisible;
}

void EditorWindowManager::SetVisibleByName(const std::string& _name, bool _visible)
{
    for (auto& [id, entry] : windows_)
    {
        if (entry.name == _name)
        {
            entry.isVisible = _visible;
            return;
        }
    }
}

void EditorWindowManager::Draw()
{
#ifdef _DEBUG

    // 描画中の登録でイテレータが壊れないよう，先にIDを控える
    std::vector<uint64_t> ids;
    ids.reserve(windows_.size());
    for (const auto& [id, entry] : windows_)
        ids.push_back(id);

    isDrawing_ = true;

    for (const uint64_t id : ids)
    {
        auto it = windows_.find(id);
        if (it == windows_.end())
            continue;

        WindowEntry& entry = it->second;
        if (!entry.isVisible)
            continue;

        if (entry.requestFocus)
        {
            ImGui::SetNextWindowFocus();
            entry.requestFocus = false;
        }

        // Raw はコールバックが自分で Begin / End を呼ぶ
        if (HasFlag(entry.flags, EditorWindowFlags::Raw))
        {
            entry.drawFunc();
            continue;
        }

        if (ImGui::Begin(entry.name.c_str(), &entry.isVisible,
                         static_cast<ImGuiWindowFlags>(entry.windowFlags)))
        {
            entry.drawFunc();
        }
        ImGui::End();
    }

    isDrawing_ = false;

    for (const uint64_t id : pendingRemove_)
        windows_.erase(id);

    pendingRemove_.clear();

#endif // _DEBUG
}

void EditorWindowManager::DrawMenu()
{
#ifdef _DEBUG

    if (windows_.empty())
    {
        ImGui::TextDisabled("(no window)");
        return;
    }

    // カテゴリごとにまとめる。ここは名前順で良い
    std::map<std::string, std::vector<uint64_t>> byCategory;
    for (const auto& [id, entry] : windows_)
        byCategory[entry.category].push_back(id);

    for (const auto& [category, ids] : byCategory)
    {
        if (!ImGui::BeginMenu(category.c_str()))
            continue;

        for (const uint64_t id : ids)
        {
            auto it = windows_.find(id);
            if (it == windows_.end())
                continue;

            WindowEntry& entry = it->second;
            if (ImGui::MenuItem(entry.name.c_str(), nullptr, &entry.isVisible))
            {
                if (entry.isVisible)
                    entry.requestFocus = true;
            }
        }

        ImGui::EndMenu();
    }

#endif // _DEBUG
}

} // namespace Engine
