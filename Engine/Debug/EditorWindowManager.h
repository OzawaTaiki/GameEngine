#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>


namespace Engine {

class EditorWindowManager;

enum class EditorWindowFlags : uint32_t
{
    None = 0,

    /// 登録直後から表示する
    DefaultOpen = 1 << 0,

    /// ImGui::Begin / End をコールバック側で行う
    /// メニューバー付きの窓など，Beginに細工が要る場合に使う
    /// このときコールバックは表示中のみ呼ばれる
    Raw = 1 << 1,
};

inline EditorWindowFlags operator|(EditorWindowFlags _a, EditorWindowFlags _b)
{
    return static_cast<EditorWindowFlags>(static_cast<uint32_t>(_a) | static_cast<uint32_t>(_b));
}

inline bool HasFlag(EditorWindowFlags _value, EditorWindowFlags _flag)
{
    return (static_cast<uint32_t>(_value) & static_cast<uint32_t>(_flag)) != 0;
}


/// <summary>
/// 登録したウィンドウの寿命を握るハンドル
/// 破棄されると自動で登録解除されるので，登録元より長生きするコールバックが残らない
/// </summary>
class EditorWindowHandle
{
public:

    EditorWindowHandle() = default;
    ~EditorWindowHandle();

    // ムーブのみ許可する
    EditorWindowHandle(EditorWindowHandle&& _other) noexcept;
    EditorWindowHandle& operator=(EditorWindowHandle&& _other) noexcept;
    EditorWindowHandle(const EditorWindowHandle&) = delete;
    EditorWindowHandle& operator=(const EditorWindowHandle&) = delete;

    /// <summary> 明示的に登録を解除する </summary>
    void Release();

    bool IsValid() const { return id_ != 0; }

    /// <summary>
    /// 実際に登録された名前
    /// 同名が既にある場合は連番が付くので，指定した名前と一致するとは限らない
    /// </summary>
    const std::string& GetName() const { return name_; }

    void SetVisible(bool _visible);
    bool IsVisible() const;
    void Focus();

private:
    friend class EditorWindowManager;

    EditorWindowHandle(uint64_t _id, const std::string& _name) : id_(_id), name_(_name) {}

    uint64_t id_ = 0;
    std::string name_;
};


/// <summary>
/// 自前のImGuiウィンドウを登録して，メニューからの表示切り替えと描画をまとめて面倒見るクラス
/// </summary>
class EditorWindowManager
{
public:

    static EditorWindowManager* GetInstance();

    /// <summary>
    /// ウィンドウを登録する
    /// 戻り値のハンドルを保持している間だけ登録が生きる
    /// </summary>
    /// <param name="_name">ウィンドウ名 (重複時は連番が付く)</param>
    /// <param name="_drawFunc">中身を描くコールバック</param>
    /// <param name="_category">メニューでのグループ名</param>
    /// <param name="_flags">EditorWindowFlags</param>
    /// <param name="_windowFlags">ImGuiWindowFlags (Raw指定時は無視される)</param>
    [[nodiscard]]
    EditorWindowHandle Register(const std::string& _name,
                                std::function<void()> _drawFunc,
                                const std::string& _category = "Custom",
                                EditorWindowFlags _flags = EditorWindowFlags::None,
                                int32_t _windowFlags = 0);

    /// <summary>
    /// アプリの終了まで登録しっぱなしにする
    /// シングルトンなど，明示的に解除する必要が無いものだけに使うこと
    /// </summary>
    /// <returns>実際に登録された名前</returns>
    std::string RegisterPersistent(const std::string& _name,
                                   std::function<void()> _drawFunc,
                                   const std::string& _category = "Custom",
                                   EditorWindowFlags _flags = EditorWindowFlags::None,
                                   int32_t _windowFlags = 0);

    /// <summary> 登録されているウィンドウをすべて描画する </summary>
    void Draw();

    /// <summary> メニューバーに項目を並べる。BeginMainMenuBar の中で呼ぶこと </summary>
    void DrawMenu();

    void SetVisible(uint64_t _id, bool _visible);
    bool IsVisible(uint64_t _id) const;

    /// <summary> 次に描画されるときに手前に持ってくる </summary>
    void RequestFocus(uint64_t _id);

    /// <summary> 名前で表示状態を変える。見つからなければ何もしない </summary>
    void SetVisibleByName(const std::string& _name, bool _visible);

    void Unregister(uint64_t _id);

    size_t GetWindowCount() const { return windows_.size(); }

private:

    struct WindowEntry
    {
        std::string name;
        std::string category;
        std::function<void()> drawFunc;
        EditorWindowFlags flags = EditorWindowFlags::None;
        int32_t windowFlags = 0;
        bool isVisible = false;
        // 次のフレームで ImGui::SetWindowFocus する
        bool requestFocus = false;
    };

    uint64_t RegisterInternal(const std::string& _name,
                              std::function<void()> _drawFunc,
                              const std::string& _category,
                              EditorWindowFlags _flags,
                              int32_t _windowFlags,
                              std::string& _outName);

    /// <summary> 既に使われている名前なら連番を付けて返す </summary>
    std::string MakeUniqueName(const std::string& _name) const;

    // IDは単調増加なので map の走査順 = 登録順になる
    std::map<uint64_t, WindowEntry> windows_;
    uint64_t nextID_ = 1;

    // 描画中の解除でコールバックが消えないように遅延させる
    bool isDrawing_ = false;
    std::vector<uint64_t> pendingRemove_;

private:
    EditorWindowManager() = default;
    ~EditorWindowManager() = default;
    // コピー禁止
    EditorWindowManager(const EditorWindowManager&) = delete;
    EditorWindowManager& operator=(const EditorWindowManager&) = delete;

};

} // namespace Engine
