#pragma once

#include <Features/Scene/Interface/BaseScene.h>
#include <Features/Camera/Camera/Camera.h>
#include <Features/Camera/DebugCamera/DebugCamera.h>
#include <System/Input/Input.h>
#include <Features/Effect/Manager/ParticleSystem.h>
#include <Features/Effect/Effect/Effect.h>
#include <Features/Light/Group/LightGroup.h>
#include <Features/Model/SkyBox.h>
#include <Features/Model/Primitive/Creater/PrimitiveCreator.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <Features/Sprite/Sprite.h>

class EffectEditorScene : public BaseScene
{
public:
    ~EffectEditorScene() override;

    void Initialize(SceneData* _sceneData) override;
    void Update() override;
    void Draw() override;
    void DrawShadow() override;

private:
    // === カメラ関連 ===
    Camera sceneCamera_ = {};
    DebugCamera debugCamera_ = {};
    bool enableDebugCamera_ = false;

    // === システム関連 ===
    Input* input_ = nullptr;
    ParticleSystem* particleSystem_ = nullptr;
    std::shared_ptr<LightGroup> lights_;

    std::vector<std::unique_ptr<ObjectModel>> objectModels_;
    std::unique_ptr<Sprite> backgroundSprite_ = nullptr;
    // === エフェクト管理 ===
    std::vector<std::unique_ptr<Effect>> effects_;
    int selectedEffectIndex_ = -1;
    int selectedEmitterIndex_ = -1;

    // === UI状態 ===
    bool showEffectLibrary_ = true;
    bool showEffectEditor_ = true;
    bool showEmitterEditor_ = true;
    bool showPreviewControls_ = true;
    bool showPerformanceMonitor_ = true;

    // === エフェクト作成用 ===
    char newEffectNameBuf_[256] = "NewEffect";
    char newEmitterNameBuf_[256] = "NewEmitter";
    char savePresetNameBuf_[256] = {};
    char loadPresetNameBuf_[256] = {};

    // === プレビュー制御 ===
    bool isPlaying_ = false;
    bool isPaused_ = false;
    bool isLooping_ = false;
    float playbackSpeed_ = 1.0f;
    float previewTime_ = 0.0f;

    // === カメラ制御 ===
    Vector3 cameraTargetPosition_ = { 0, 0, 0 };
    float cameraDistance_ = 10.0f;
    float cameraHeight_ = 2.0f;
    float cameraRotationY_ = 0.0f;
    bool autoRotateCamera_ = false;

    // === グリッド・ガイド ===
    bool showGrid_ = true;
    bool showAxis_ = true;
    bool showBounds_ = false;
    float gridSize_ = 20.0f;
    int gridCount_ = 20;

    // === テンプレート・プリセット ===
    struct EffectTemplate {
        std::string name;
        std::string description;
        std::vector<std::string> emitterTemplates;
    };
    std::vector<EffectTemplate> effectTemplates_;

    // === パフォーマンス監視 ===
    struct PerformanceData {
        float updateTime = 0.0f;
        float drawTime = 0.0f;
        uint32_t totalParticles = 0;
        uint32_t activeParticles = 0;
        uint32_t drawCalls = 0;
        float frameTime = 0.0f;
    };
    PerformanceData performanceData_;
    std::vector<float> frameTimeHistory_;
    static const size_t kFrameHistorySize = 120; // 2秒分のフレーム履歴

    // === ファイル操作 ===
    std::string currentProjectPath_ = "Resources/Data/Effects/";
    std::vector<std::string> recentFiles_;
    static const size_t kMaxRecentFiles = 10;

    // === 環境・背景 ===
    std::unique_ptr<SkyBox> skyBox_ = nullptr;
    Vector4 backgroundColor_ = { 0.1f, 0.1f, 0.15f, 1.0f };
    bool usesSkyBox_ = true;

    // === ホットキー ===
    struct HotKeys {
        bool playPause = false;  // Space
        bool stop = false;       // S
        bool reset = false;      // R
        bool save = false;       // Ctrl+S
        bool load = false;       // Ctrl+O
        bool newEffect = false;  // Ctrl+N
    };
    HotKeys hotKeys_;

    PrimitiveCreator primitiveCreator_;

#ifdef _DEBUG
    // === ImGui関数 ===
    void ImGuiMainMenuBar();
    void ImGuiEffectLibrary();
    void ImGuiEffectEditor();
    void ImGuiEmitterEditor();
    void ImGuiPreviewControls();
    void ImGuiPerformanceMonitor();
    void ImGuiCameraControls();
    void ImGuiEnvironmentSettings();
    void ImGuiModelControls();
    void ImGuiTemplateSelector();
    void ImGuiFileOperations();
    void ImGuiHelpWindow();

    // === ヘルパー関数 ===
    void CreateEffectFromTemplate(const std::string& _templateName);
    void CreateEmitterFromTemplate(const std::string& _templateName);
    void UpdatePerformanceData();
    void UpdateCamera();
    void ProcessHotKeys();
    void DrawGrid();
    void DrawAxis();
    void DrawBounds();

    // === ファイル操作ヘルパー ===
    bool SaveCurrentEffect();
    bool LoadEffectFromFile(const std::string& _filePath);
    bool SaveProject(const std::string& _filePath);
    bool LoadProject(const std::string& _filePath);
    void AddToRecentFiles(const std::string& _filePath);

    // === テンプレート初期化 ===
    void InitializeTemplates();

    // === エラー処理 ===
    void ShowErrorDialog(const std::string& _message);
    void ShowInfoDialog(const std::string& _message);

    // === プレビュー制御ヘルパー ===
    void PlayCurrentEffect();
    void StopCurrentEffect();
    void PauseCurrentEffect();
    void ResetCurrentEffect();
    void SetPlaybackSpeed(float _speed);

    // === エフェクト管理ヘルパー ===
    Effect* GetCurrentEffect();
    ParticleEmitter* GetCurrentEmitter();
    void SelectEffect(int _index);
    void SelectEmitter(int _index);
    void DuplicateCurrentEffect();
    void DeleteCurrentEffect();

    // === プロパティ操作 ===
    void CopyEmitterProperties();
    void PasteEmitterProperties();
    bool hasClipboardData_ = false;

    // === ドラッグ&ドロップ ===
    void HandleFileDragDrop();

    // === ツールチップ・ヘルプ ===
    void ShowTooltip(const std::string& _text);
    void ShowContextHelp(const std::string& _topic);
#endif
};