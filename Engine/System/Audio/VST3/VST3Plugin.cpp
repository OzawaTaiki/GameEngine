#include "VST3Plugin.h"

/*
memo
classInfo : DLLの中にあるプラグインの1つを表す情報
    1つのvst3の中に複数のプラグインが複数含まれていることもある
    classInfo.ID()はこれを識別するためのUUID

PluginFactory: DLLの中にあるプラグインを作成するための工場
    createInstance<T>(UUID)で、UUIDに対応するプラグインのT型のインスタンスを作成することができる
    例えば、createInstance<IComponent>(UUID)で、UUIDに対応するプラグインのIComponent型のインスタンスを作成することができる

*/

bool Engine::VST3Plugin::Load(VST3::Hosting::PluginFactory factory, const VST3::Hosting::ClassInfo& classInfo)
{
    // UUIDからプラグインのIComponentを作成する
    auto result = factory.createInstance<Steinberg::Vst::IComponent>(classInfo.ID());

    // プラグインの作成に失敗した場合はfalseを返す
    if (!result)
    {
        return false;
    }

    //  ICompとnameを保存する
    component_ = result;
    name_ = classInfo.name();

    // 成功した場合はtrueを返す
    return true;
}

/*
HostApplication: ホスト名やバージョンを提供するオブジェクト
        ホストによって挙動の変わるプラグインもあるらしい
        オーディオホスト（DAWなど）からプラグインに提供されるインターフェースで、ホストの情報や機能をプラグインが利用できるようにするもの
*/

bool Engine::VST3Plugin::Initialize(Steinberg::Vst::HostApplication* host, float sampleRate, int32_t maxSamplePerBlock, int32_t inputChannels, int32_t outputChannels)
{
    // IComponentが存在しない場合は
    // 初期化できないのでfalseを返す
    if (!component_)
        return false;

    inputChannels_ = inputChannels;
    outputChannels_ = outputChannels;

    // IComponentを初期化する
    auto result = component_->initialize(host);
    // 初期化に失敗した場合はfalseを返す
    if (result != Steinberg::kResultOk)
        return false;

    processor_ = Steinberg::FUnknownPtr<Steinberg::Vst::IAudioProcessor>(component_);
    if (!processor_)
        return false;

    controller_ = Steinberg::FUnknownPtr<Steinberg::Vst::IEditController>(component_);

    // 設定
    Steinberg::Vst::ProcessSetup setup{};
    setup.processMode           = Steinberg::Vst::kRealtime; // リアルタイムorオフライン処理モード
    setup.symbolicSampleSize    = Steinberg::Vst::kSample32; // サンプルサイズ（32ビット浮動小数点）
    setup.maxSamplesPerBlock    = maxSamplePerBlock; // 一回に処理する最大サンプル数
    setup.sampleRate            = sampleRate; // サンプルレート

    // プロセッサーに設定を適用する
    if (processor_->setupProcessing(setup) != Steinberg::kResultOk)
        return false;
    // プロセッサーをアクティブにする
    if (component_->setActive(true) != Steinberg::kResultOk)
        return false;
    // プロセッサーを処理状態にする
    if (processor_->setProcessing(true) != Steinberg::kResultOk)
        return false;

    isActive_ = true;
    return true;
}

void Engine::VST3Plugin::Terminate()
{
    if (!isActive_)
        return;

    processor_->setProcessing(false);
    component_->setActive(false);
    component_->terminate();
    isActive_ = false;
}

/*
ProcessData: 処理用データ
    inputs → 入力音声バッファ（planar形式）
    outputs → 出力音声バッファ（planar形式）
    numSamples → 今回処理するサンプル数
    inputParameterChanges → パラメータ変更情報
    processContext → テンポや再生位置などの情報

planar形式: LLLLL...L RRRRRR...(VST3)
    例えばステレオの場合、左チャンネルのサンプルが連続して格納され、その後に右チャンネルのサンプルが連続して格納される形式

インターリーブ形式: L R L R L R ... (XAudio2)
    例えばステレオの場合、左チャンネルと右チャンネルのサンプルが交互に格納される形式

*/

bool Engine::VST3Plugin::Process(Steinberg::Vst::ProcessData& data)
{
    if (!isActive_ || !processor_)
        return false;

    // オーディオデータ処理
    auto result = processor_->process(data);
    // process: オーディオデータを処理する関数
    return result == Steinberg::kResultOk;
}
