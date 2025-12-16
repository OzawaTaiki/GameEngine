#pragma once

#include <cstdint>
#include <string>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif


/**
 * @brief システム時間を管理するクラス
 *
 * フレーム間の時間計測、フレームレート制御などの
 * 基本的な時間管理機能を提供します。
 */
class Time
{
public:

    /**
    * @brief 初期化
    * @brief メンバ変数の初期化を行う
    */
    static void Initialize();

    /*
    * @brief 更新
    * @brief 現在時間を取得し、デルタタイムを更新する
    */
    static void Update();

    /*
    * @brief デルタタイム取得
    * @return 前フレームからの経過時間
    */
    template<typename T>
    static T GetDeltaTime() { return static_cast<T>(deltaTime_); }

    /**
     * @brief 起動からの総経過時間を取得
     * @return 総経過時間（秒）
     */
    static double GetTotalTime() { return totalTime_; }

    /**
     * @brief 現在のフレームレートを取得
     * @return フレームレート（FPS）
     */
    static double GetFramerate() { return framerate_; }

    /**
     * @brief 現在時刻を取得
     * @return システム起動からの経過時間（秒）
     */
    static double GetCurrentTime();

    /**
    * @brief デルタタイムを固定値にするかどうかを設定
    * @param _isFixed trueの場合、デルタタイムを固定値にする
    */
    static void SetDeltaTimeFixed(bool _isFixed) { isDeltaTimeFixed_ = _isFixed; }

    /**
     * @brief デフォルトのフレームレートを設定
     * @param _framerate フレームレート（FPS）
     */
    static void SetDefaultFramerate(float _framerate) { defaultFramerate_ = _framerate; }

    /**
    * @brief デルタタイムが固定値かどうかを取得
    * @return デルタタイムが固定値の場合true
    */
    static bool IsDeltaTimeFixed() { return isDeltaTimeFixed_; }

    static void TimeStamp(const std::string& _label = "");

#ifdef _DEBUG
    static void ImGui(bool* _open);
#endif // _DEBUG
private:

    static double deltaTime_;           ///<! デルタタイム
    static double updateInterval_;      ///<! デルタタイムの更新間隔
    static bool isDeltaTimeFixed_;      ///<! デルタタイムを固定するかどうか
    static double totalTime_;           ///<! 総時間
    static double prevTime_;            ///<! 前回の時間
    static double currentTime_;         ///<! 現在の時間
    static int32_t frameCount_;         ///<! フレーム数
    static double framerate_;           ///<! フレームレート
    static float defaultFramerate_;     ///<! デフォルトのフレームレート

};
