#include "SequenceEvent.h"

void SequenceEvent::Update(float _currentTime)
{
    // 時間が範囲外のときは早期リターン
    if (_currentTime < startTime_ || _currentTime > startTime_ + duration_)
    {
        return;
    }


    // 現在の時間を正規化
    float normalizedTime = (_currentTime - startTime_) / duration_;

    // イージング処理
    float t = Easing::SelectFuncPtr(easingType_)(normalizedTime);

    // 値の適用


}
