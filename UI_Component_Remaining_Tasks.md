# UI Component設計 残りの作業内容

## 完了済み
- ✅ Phase 1: UIElement, UIComponent基底クラス
- ✅ Phase 2-1: ColliderComponent
- ✅ Phase 2-2: SpriteRenderComponent, TextRenderComponent

---

## Phase 3: UIButton実装（統合テスト）（1日）

### 目的
基本的なボタンUIを実装し、Component設計の動作検証を行う

### 3-1. ButtonComponent実装（3-4時間）

**ファイル**: `UI/Component/ButtonComponent.h`

```cpp
#pragma once
#include "UIComponent.h"
#include <functional>

class ColliderComponent;

enum class ButtonState {
    Normal,
    Hover,
    Pressed,
    Disabled
};

class ButtonComponent : public UIComponent {
public:
    ButtonComponent() = default;
    ~ButtonComponent() override = default;
    
    void Initialize() override;
    void Update() override;
    void DrawImGui() override;
    
    void SetOnClick(std::function<void()> callback) { onClick_ = callback; }
    void SetOnHover(std::function<void()> callback) { onHover_ = callback; }
    
    ButtonState GetState() const { return state_; }
    void SetEnabled(bool enabled);
    
private:
    ColliderComponent* collider_ = nullptr;
    ButtonState state_ = ButtonState::Normal;
    
    std::function<void()> onClick_;
    std::function<void()> onHover_;
    
    void UpdateState();
};
```

**実装内容**:
- Initialize()でColliderComponentを取得/自動追加
- Update()でマウス位置を取得し、状態遷移を管理
- Normal → Hover → Pressed の状態管理
- onClick_コールバックの呼び出し

**確認項目**:
- [ ] Collider自動追加
- [ ] Hover状態の検知
- [ ] Click検知とコールバック実行
- [ ] Disabled状態の動作

---

### 3-2. UIButton実装（2-3時間）

**ファイル**: `UI/UIButton.h`, `UI/UIButton.cpp`

```cpp
class UIButton : public UIElement {
public:
    UIButton(const Vector2& pos, const Vector2& size, 
             const std::wstring& text = L"Button");
    ~UIButton() override = default;
    
    void SetOnClick(std::function<void()> callback);
    void SetText(const std::wstring& text);
    
private:
    SpriteRenderComponent* background_ = nullptr;
    TextRenderComponent* text_ = nullptr;
    ButtonComponent* button_ = nullptr;
    
    void OnStateChanged(ButtonState state);
};
```

**実装内容**:
- コンストラクタで必要なComponentを追加
  - SpriteRenderComponent（背景）
  - TextRenderComponent（テキスト）
  - ButtonComponent（インタラクション）
  - ColliderComponent（自動追加される）
- 状態に応じた色変更
- テキストの中央揃え

**確認項目**:
- [ ] ボタン生成
- [ ] クリック動作
- [ ] Hover時の色変更
- [ ] テキスト表示
- [ ] ImGuiでパラメータ調整

---

## Phase 4: Navigation実装（1日）

### 目的
キーボード/ゲームパッドでのUI要素間のフォーカス移動を実装

### 4-1. NavigationComponent実装（3-4時間）

**ファイル**: `UI/Component/NavigationComponent.h`

```cpp
#pragma once
#include "UIComponent.h"
#include <memory>

enum class Direction {
    Up,
    Down,
    Left,
    Right
};

class NavigationComponent : public UIComponent {
public:
    NavigationComponent() = default;
    ~NavigationComponent() override = default;
    
    void Initialize() override;
    void DrawImGui() override;
    
    void SetNavigation(Direction dir, UIElement* target);
    UIElement* GetNext(Direction dir) const;
    
    void SetFocusable(bool focusable) { focusable_ = focusable; }
    bool IsFocusable() const { return focusable_; }
    
private:
    std::weak_ptr<UIElement> up_;
    std::weak_ptr<UIElement> down_;
    std::weak_ptr<UIElement> left_;
    std::weak_ptr<UIElement> right_;
    
    bool focusable_ = true;
};
```

**実装内容**:
- 4方向のNavigation設定
- weak_ptrで循環参照を防止
- ImGuiでNavigation先の表示

**確認項目**:
- [ ] Navigation設定
- [ ] GetNext()で正しい要素を取得
- [ ] 無効なポインタの処理

---

### 4-2. NavigationManager実装（2-3時間）

**ファイル**: `UI/NavigationManager.h`

```cpp
class NavigationManager {
public:
    static NavigationManager* GetInstance();
    
    void Register(UIElement* element);
    void Unregister(UIElement* element);
    
    void HandleInput();
    void SetFocus(UIElement* element);
    UIElement* GetFocus() const { return currentFocus_; }
    
private:
    NavigationManager() = default;
    
    std::vector<UIElement*> focusableElements_;
    UIElement* currentFocus_ = nullptr;
};
```

**実装内容**:
- シングルトンパターン
- focusable要素の登録管理
- 方向キー入力の処理
- Enterキーで決定（ButtonのonClick実行）

**確認項目**:
- [ ] 要素の登録/登録解除
- [ ] 方向キーでフォーカス移動
- [ ] Enterキーで決定
- [ ] フォーカスの視覚的フィードバック

---

### 4-3. UIButtonにNavigation統合（1時間）

**修正内容**:
- UIButtonコンストラクタにNavigationComponent追加
- NavigationManagerに自動登録
- フォーカス時の視覚的フィードバック（枠線など）

**確認項目**:
- [ ] 複数ボタン間の移動
- [ ] ループ構造
- [ ] フォーカス表示

---

## Phase 5: UISlider実装（複合UI検証）（1-2日）

### 目的
複数の子要素を持つ複合UIの実装パターンを確立

### 5-1. SliderValueComponent実装（4-5時間）

**ファイル**: `UI/Component/SliderValueComponent.h`

```cpp
class SliderValueComponent : public UIComponent {
public:
    SliderValueComponent() = default;
    ~SliderValueComponent() override = default;
    
    void Initialize() override;
    void Update() override;
    void DrawImGui() override;
    
    void SetTrack(UIElement* track);
    void SetHandle(UIElement* handle);
    
    void SetValue(float value);
    float GetValue() const { return value_; }
    
    void SetRange(float min, float max);
    void SetOnValueChanged(std::function<void(float)> callback);
    
private:
    UIElement* track_ = nullptr;
    UIElement* handle_ = nullptr;
    
    float value_ = 0.5f;
    float minValue_ = 0.0f;
    float maxValue_ = 1.0f;
    
    bool isDraggingHandle_ = false;
    
    std::function<void(float)> onValueChanged_;
    
    void UpdateValueFromPosition(const Vector2& mousePos);
    void UpdateHandlePosition();
};
```

**実装内容**:
- トラック（バー）とハンドル（つまみ）への参照
- ハンドルのドラッグ処理
- トラッククリックでハンドル移動
- 値の計算とハンドル位置の更新
- onValueChangedコールバック

**確認項目**:
- [ ] ハンドルのドラッグ
- [ ] トラッククリック
- [ ] 値の範囲制限
- [ ] コールバック実行

---

### 5-2. UISlider実装（3-4時間）

**ファイル**: `UI/UISlider.h`, `UI/UISlider.cpp`

```cpp
class UISlider : public UIElement {
public:
    UISlider(const Vector2& pos, const Vector2& size);
    ~UISlider() override = default;
    
    void SetValue(float value);
    float GetValue() const;
    
    void SetRange(float min, float max);
    void SetOnValueChanged(std::function<void(float)> callback);
    
private:
    UIElement* track_ = nullptr;
    UIElement* handle_ = nullptr;
    SliderValueComponent* sliderValue_ = nullptr;
};
```

**実装内容**:
- トラック作成（UIElement + SpriteRender + Collider）
- ハンドル作成（UIElement + SpriteRender + Collider）
- 子要素として追加
- SliderValueComponent追加
- NavigationComponent追加

**確認項目**:
- [ ] スライダー生成
- [ ] ドラッグ操作
- [ ] トラッククリック
- [ ] Navigation対応
- [ ] 値の保存/読込（Phase 6で）

---

## Phase 6: SaveableComponent実装（半日）

### 目的
UIパラメータの保存/読込機能を提供

### 6-1. SaveableComponent実装（3-4時間）

**ファイル**: `UI/Component/SaveableComponent.h`

```cpp
class SaveableComponent : public UIComponent {
public:
    SaveableComponent(const std::string& key = "");
    ~SaveableComponent() override = default;
    
    void Initialize() override;
    void DrawImGui() override;
    
    void SetKey(const std::string& key) { key_ = key; }
    void SetDefaultValue(float value) { defaultValue_ = value; }
    void SetRange(float min, float max);
    
    void SetValue(float value);
    float GetValue() const { return value_; }
    
    void Save();
    void Load();
    
private:
    std::string key_;
    float value_ = 0.0f;
    float defaultValue_ = 0.0f;
    float minValue_ = 0.0f;
    float maxValue_ = 1.0f;
    
    std::string GetFilePath() const;
};
```

**実装内容**:
- JSON形式でファイル保存
- ファイル存在チェック
- なければdefaultValue_を使用
- Initialize時に自動Load

**確認項目**:
- [ ] ファイル保存
- [ ] ファイル読込
- [ ] デフォルト値
- [ ] 複数パラメータ

---

### 6-2. UISliderにSaveable統合（1時間）

**修正内容**:
- UISliderコンストラクタにsaveKey引数追加
- SaveableComponent追加
- SliderValueComponentとの連携

**確認項目**:
- [ ] スライダー値の保存
- [ ] 起動時の読込

---

## Phase 7: 残りのUI実装（1-2日）

### 7-1. UIText実装（1時間）

```cpp
class UIText : public UIElement {
public:
    UIText(const Vector2& pos, const std::wstring& text);
    
    void SetText(const std::wstring& text);
};
```

---

### 7-2. UIImage実装（1時間）

```cpp
class UIImage : public UIElement {
public:
    UIImage(const Vector2& pos, const Vector2& size, TextureID texture);
    
    void SetTexture(TextureID texture);
};
```

---

### 7-3. UIPanel実装（1時間）

```cpp
class UIPanel : public UIElement {
public:
    UIPanel(const Vector2& pos, const Vector2& size, bool hasBackground = true);
};
```

---

### 7-4. UISliderWithInput実装（3-4時間）

**複合UI**:
- UISlider
- UITextBox（入力欄）
- 値の同期処理

---

## Phase 8: 最適化・仕上げ（1-2日）

### 8-1. パフォーマンス最適化

- [ ] Component検索の最適化
- [ ] 描画順の最適化
- [ ] メモリ使用量チェック

---

### 8-2. デバッグ機能強化

- [ ] ImGuiツリー表示
- [ ] Component一覧
- [ ] 階層構造の可視化
- [ ] エラーログ
- [ ] 当たり判定の可視化

---

### 8-3. ドキュメント作成

- [ ] 使い方ドキュメント
- [ ] 設計ドキュメント
- [ ] サンプルコード

---

## 作業スケジュール概算

| Phase | 作業内容 | 推定時間 |
|-------|---------|---------|
| Phase 3 | UIButton実装 | 1日 |
| Phase 4 | Navigation実装 | 1日 |
| Phase 5 | UISlider実装 | 1-2日 |
| Phase 6 | Saveable実装 | 半日 |
| Phase 7 | 残りのUI | 1-2日 |
| Phase 8 | 最適化・仕上げ | 1-2日 |
| **合計** | | **6-9日** |

---

## 次にやること

**Phase 3: UIButton実装**から始めてください。

1. ButtonComponent.h/cpp を実装
2. UIButton.h/cpp を実装
3. テストして動作確認
4. 完了したら報告

準備ができたら「次の指示書をください」と言ってください！
