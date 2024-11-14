#define NOMINMAX
#include "CatmulRomSpline.h"
#include "TextureManager.h"
#include "Camera.h"
#include "VectorFunction.h"
#include "MyLib.h"
#include "imgui.h"
#include "LineDrawer.h"
#include "Input.h"
#include <optional>

CatmulRomSpline::CatmulRomSpline()
{
}

CatmulRomSpline::~CatmulRomSpline()
{
#ifdef _DEBUG
    ConvertToFinalDataOfRotCtrl();
    jsonLoader_->PrepareForSave();
    // jsonにsave
    jsonLoader_->SetData<Vector3>("posCtrlPoint", "pos", finalPosCtrlPoints_);

    std::vector<Vector3> rot;
    std::vector<float> pos;
    for (auto rotP : finalRotCtrlPoints_)
    {
        rot.push_back(rotP.rotate);
        pos.push_back(rotP.posOnLine);
    }

    jsonLoader_->SetData("rotCtrlPoint", "rotate", rot);
    jsonLoader_->SetData("rotCtrlPoint", "posOnLine", pos);
#endif // _DEBUG

    delete jsonLoader_;
    delete colorRed_;
    delete colorWhite_;
    delete moveObjColor_;

    return;
}

void CatmulRomSpline::Initialize(const std::string& _filePath)
{
    filepath_ = _filePath;

    jsonLoader_ = new JsonLoader;
    jsonLoader_->LoadJson(filepath_, true);

    posModel_ = Model::CreateFromObj("sphere/sphere.obj");
    rotModel_ = Model::CreateFromObj("axis/axis.gltf");

    moveObjTrans_.Initialize();
    moveObjTrans_.scale_ = { 0.2f,0.2f ,0.2f };
    moveObjTrans_.UpdateData();

    moveObjColor_ = new ObjectColor;
    moveObjColor_->Initialize();
    moveObjColor_->SetColor({ 1, 1, 1, 0.6f });

    //rotModelTexture_ = TextureManager::GetInstance()->Load("axisCube.png");


    colorWhite_ = new ObjectColor;
    colorRed_ = new ObjectColor;

    colorWhite_->Initialize();
    colorWhite_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

    colorRed_->Initialize();
    colorRed_->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });

    // 念のためクリア
    areaLength_.clear();
    cumulativeLength_.clear();

    // finalに保存
    finalPosCtrlPoints_ = jsonLoader_->GetDatumArray<Vector3>("posCtrlPoint", "pos").value();
    auto rotateData = jsonLoader_->GetDatumArray<Vector3>("rotCtrlPoint", "rotate").value();
    auto posOnLine = jsonLoader_->GetDatumArray<float>("rotCtrlPoint", "posOnLine").value();
    CreateFinalRotData(rotateData, posOnLine);

    // 読み込んだデータをedit配列に
    ConvertToEditableData();

    // endを入れて選択していないことに
    selectPosIterator_ = editPosCtrlPoints_.end();
    selectRotIterator_ = editRotCtrlPoints_.end();

    // pos配列が空のときposを追加
    if (editPosCtrlPoints_.empty())
    {
        AddPosCtrlPoint();
    }

    // データがあるときはそれらでスプラインを計算
    else
    {
        CalculateCatmulRomSpline();
    }

    // rotのデータがないときデータを追加
    if (editRotCtrlPoints_.empty())
    {
        AddRotCtrlPoint();
    }

#ifdef _DEBUG
    isDrawCtrlPoint_ = true;
    isDrawPosCtrlPoint_ = true;
    isDrawRotCtrlPoint_ = true;
#else
    isDrawCtrlPoint_ = false;
    isDrawPosCtrlPoint_ = false;
    isDrawRotCtrlPoint_ = false;
#endif // _DEBUG
    isMove_ = false;


    deltaTime_ = 1.0f / 60.0f;

    speed_ = 2.0f;
    hitRadius_ = 20.0f;

    return;
}

void CatmulRomSpline::Update(const Matrix4x4& _vp)
{
    isChangeCtrlPoint_ = false;
    ImGui();

    if (isChangeCtrlPoint_)
        CalculateCatmulRomSpline();

    if (Input::GetInstance()->IsMouseTriggered(0))
    {
        SelectPoint(_vp);
    }


    if (isMove_ && pMoveObj_)
        CalculatePositinByPosOnLine();

    for (const auto& wt : editPosCtrlPoints_) {
        wt->worldTransform.UpdateData();
    }
    for (const auto& wt : editRotCtrlPoints_) {
        wt->worldTransform.transform_ = GetPositionByLength(wt->posOnLine);
        wt->worldTransform.UpdateData();
    }
    RegisterDrawPoint();

}

void CatmulRomSpline::Draw(const Camera* _camera)
{
#ifndef _DEBUG
    return;
#endif // DEBUG

    if (isDrawCtrlPoint_)
    {
        if (isDrawPosCtrlPoint_)
        {
            for (const auto& ctrlPoint : editPosCtrlPoints_)
            {
                if (selectPosIterator_ != editPosCtrlPoints_.end() &&
                    ctrlPoint.get() == (*selectPosIterator_).get())
                    posModel_->Draw(ctrlPoint->worldTransform, _camera, posModelTexture_, colorRed_);
                else
                    posModel_->Draw(ctrlPoint->worldTransform, _camera, posModelTexture_, colorWhite_);
            }
        }
        if (isDrawRotCtrlPoint_)
        {
            for (const auto& ctrlPoint : editRotCtrlPoints_)
            {
                if (selectRotIterator_ != editRotCtrlPoints_.end() &&
                    ctrlPoint.get() == (*selectRotIterator_).get())
                    rotModel_->Draw(ctrlPoint->worldTransform, _camera, colorRed_);
                else
                    rotModel_->Draw(ctrlPoint->worldTransform, _camera, colorWhite_);
            }
        }
    }
}

void CatmulRomSpline::CreateFinalRotData(const std::vector<Vector3>& _rotArr, const std::vector<float>& _posArr)
{
    // サイズが一緒前提
    assert(_rotArr.size() == _posArr.size());

    // 配列の長さを取得
    size_t length = _rotArr.size();

    // サイズ分
    for (size_t index = 0; index < length; index++)
    {
        RotatePoint rotP = { _rotArr[index],_posArr[index] };
        finalRotCtrlPoints_.push_back(rotP);
    }

    return;
}

void CatmulRomSpline::ConvertToEditableData()
{
    if (finalPosCtrlPoints_.empty())
        return;

    editPosCtrlPoints_.clear();
    for (const auto& posCtrl : finalPosCtrlPoints_)
    {
        editPosCtrlPoints_.emplace_back(std::make_unique<ControlPoint>());
        editPosCtrlPoints_.back()->number = static_cast<uint32_t>(editPosCtrlPoints_.size() - 1);
        editPosCtrlPoints_.back()->worldTransform.Initialize();
        editPosCtrlPoints_.back()->worldTransform.transform_ = posCtrl;
        editPosCtrlPoints_.back()->worldTransform.scale_ = { 0.3f };
        editPosCtrlPoints_.back()->worldTransform.UpdateData();
    }

    if (editPosCtrlPoints_.size() >= 4)
    {
        GenerateDrawPoints();
        CalculateLengths();
    }


    if (finalRotCtrlPoints_.empty())
        return;

    editRotCtrlPoints_.clear();
    for (const auto& rotCtrl : finalRotCtrlPoints_)
    {
        editRotCtrlPoints_.emplace_back(std::make_unique<ControlPoint>());

        editRotCtrlPoints_.back()->number = static_cast<uint32_t>(editRotCtrlPoints_.size() - 1);
        editRotCtrlPoints_.back()->posOnLine = rotCtrl.posOnLine;
        editRotCtrlPoints_.back()->worldTransform.Initialize();
        editRotCtrlPoints_.back()->worldTransform.transform_ = GetPositionByLength(rotCtrl.posOnLine);
        editRotCtrlPoints_.back()->worldTransform.rotate_ = rotCtrl.rotate;
        editRotCtrlPoints_.back()->worldTransform.scale_ = { 0.3f };
        editRotCtrlPoints_.back()->worldTransform.UpdateData();
    }

    return;
}

void CatmulRomSpline::ConvertToFinalDataOfPosCtrl()
{
    finalPosCtrlPoints_.clear();
    for (auto it = editPosCtrlPoints_.begin(); it != editPosCtrlPoints_.end(); it++) {
        finalPosCtrlPoints_.push_back(it->get()->worldTransform.GetWorldPosition());
    }
}

void CatmulRomSpline::ConvertToFinalDataOfRotCtrl()
{
    finalRotCtrlPoints_.clear();
    for (auto it = editRotCtrlPoints_.begin(); it != editRotCtrlPoints_.end(); it++) {
        RotatePoint rp;
        rp.rotate = it->get()->worldTransform.rotate_;
        rp.posOnLine = it->get()->posOnLine;
        finalRotCtrlPoints_.push_back(rp);
    }

    return;
}

Vector3 CatmulRomSpline::GetPositionByLength(float _length)
{
    float t = GetPositionParameterForDistance(_length);
    Vector3 addPos = CalculateCatmulRomPoint(t);
    return addPos;
}

float CatmulRomSpline::GetPositionParameterForDistance(float _length)
{
    if (cumulativeLength_.size() == 0)
        return 0.0f;

    for (size_t i = 0; i < cumulativeLength_.size() - 1; ++i) {
        if (cumulativeLength_[i] <= _length && cumulativeLength_[i + 1] >= _length) {
            float t1 = static_cast<float>(i) / (cumulativeLength_.size() - 1);
            float t2 = static_cast<float>(i + 1) / (cumulativeLength_.size() - 1);
            float segmentLength = cumulativeLength_[i + 1] - cumulativeLength_[i];
            float localT = (_length - cumulativeLength_[i]) / segmentLength;
            return t1 + localT * (t2 - t1);
        }
    }
    return 1.0f; // 最後のポイントに達した場合
}

float CatmulRomSpline::GetRotateParameterForDistance(float _targetDis)
{
    int i = 0;
    for (auto it = editRotCtrlPoints_.begin(); it != editRotCtrlPoints_.end(); ++it) {
        float currentLength = it->get()->posOnLine;
        if (std::next(it) == editRotCtrlPoints_.end()) {
            if (editRotCtrlPoints_.size() == 1)
                return 0.0f;
            else
                break;
        }
        float nextLength = std::next(it)->get()->posOnLine;
        if (currentLength <= _targetDis && nextLength >= _targetDis) {
            float t1 = static_cast<float>(i) / (editRotCtrlPoints_.size() - 1);
            float t2 = static_cast<float>(i + 1) / (editRotCtrlPoints_.size() - 1);
            float segmentLength = nextLength - currentLength;
            float localT = (_targetDis - currentLength) / segmentLength;
            return t1 + localT * (t2 - t1);
        }
        i++;
    }
    return 1.0f; // 最後のポイントに達した場合
}

Vector3 CatmulRomSpline::CalculateCatmulRomPoint(float _t)
{
    ConvertToFinalDataOfPosCtrl();
    // 区間（線）の数
    size_t divisoin = finalPosCtrlPoints_.size() - 1;

    // 全体の位置から区間のインデックスを計算
    size_t index = static_cast<size_t>(std::floor(_t * divisoin));
    index = std::clamp(index, size_t(0), divisoin - 1);

    // 現在の区間内での正規化された位置（0から1の間）を計算
    float t1 = _t * divisoin - std::floor(_t * divisoin);

    size_t index0 = (index == 0) ? 0 : index - 1;
    size_t index1 = index;
    size_t index2 = std::min(index + 1, divisoin);
    size_t index3 = std::min(index + 2, divisoin);

    return CalculateCatmulRomAreaLine(index0, index1, index2, index3, t1);
}

Vector3 CatmulRomSpline::CalculateCatmulRomAreaLine(size_t _index0, size_t _index1, size_t _index2, size_t _index3, float _t)
{
    Vector3 p0 = finalPosCtrlPoints_[_index0];
    Vector3 p1 = finalPosCtrlPoints_[_index1];
    Vector3 p2 = finalPosCtrlPoints_[_index2];
    Vector3 p3 = finalPosCtrlPoints_[_index3];

    float t2 = _t * _t;
    float t3 = t2 * _t;

    Vector3 result;
    result.x = 0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * _t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);

    result.y = 0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * _t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);

    result.z = 0.5f * ((2.0f * p1.z) + (-p0.z + p2.z) * _t + (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 + (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3);

    return result;
}

void CatmulRomSpline::GenerateDrawPoints()
{

    uint32_t division = lineSegmentCount_ * static_cast<uint32_t>(editPosCtrlPoints_.size() - 1);
    float t = 0;
    lineDrawPoint_.push_back(CalculateCatmulRomPoint(t));
    for (size_t i = 1; i < division; i++)
    {
        t = 1.0f / static_cast<float>(division) * static_cast<float>(i);
        lineDrawPoint_.push_back(CalculateCatmulRomPoint(t));
    }

    RegisterDrawPoint();

    return;
}

void CatmulRomSpline::CalculateLengths()
{
    totalLength_ = 0.0f;
    areaLength_.clear();
    areaLength_.push_back(0.0f);
    float division = lineSegmentCount_ * static_cast<float>(editPosCtrlPoints_.size() - 1);
    float t = 0;
    for (size_t i = 1; i < division; i++)
    {
        areaLength_.back() += (lineDrawPoint_[i] - lineDrawPoint_[i - 1]).Length();
        if ((i + 1) % lineSegmentCount_ == 0)
        {
            totalLength_ += areaLength_.back();
            if (i != division - 1)
                areaLength_.push_back(0.0f);
        }
    }

    cumulativeLength_.clear();
    cumulativeLength_.push_back(0);
    for (size_t index = 0; index < areaLength_.size(); index++)
    {
        cumulativeLength_.push_back(cumulativeLength_.back() + areaLength_[index]);
    }

    return;
}

void CatmulRomSpline::AddPosCtrlPoint()
{
    if (selectPosIterator_ != editPosCtrlPoints_.end())
    {// 制御点を選択しているとき

        // 並べ替える
        SortPosCtrlPointByNum();

        // データの挿入
        auto it = editPosCtrlPoints_.insert(std::next(selectPosIterator_), std::make_unique<ControlPoint>());
        //初期
        (*it)->worldTransform.Initialize();
        (*it)->worldTransform.transform_ = addPosCtrlPoint_;
        (*it)->worldTransform.scale_ = { 0.3f };
        (*it)->worldTransform.UpdateData();
        (*it)->number = (*selectPosIterator_)->number + 1;

        // 以降の番号をインクリメント
        for (auto i = std::next(it); i != editPosCtrlPoints_.end(); ++i)
        {
            (*i)->number++;
        }

    }
    else
    {// 選択していないとき
        editPosCtrlPoints_.emplace_back(std::make_unique<ControlPoint>());
        editPosCtrlPoints_.back()->worldTransform.Initialize();
        editPosCtrlPoints_.back()->worldTransform.transform_ = addPosCtrlPoint_;
        editPosCtrlPoints_.back()->worldTransform.scale_ = { 0.3f };
        editPosCtrlPoints_.back()->worldTransform.UpdateData();
        editPosCtrlPoints_.back()->number = static_cast<uint32_t>(editPosCtrlPoints_.size());
    }

    return;
}

void CatmulRomSpline::AddRotCtrlPoint()
{
    if (selectRotIterator_ != editRotCtrlPoints_.end())
    {// 制御点を選択しているとき

        // 並べ替える
        SortRotCtrlPointByPos();

        // データの挿入
        auto it = editRotCtrlPoints_.insert(std::next(selectRotIterator_), std::make_unique<ControlPoint>());
        //初期
        (*it)->worldTransform.Initialize();
        (*it)->posOnLine = addRotCtrlPoint_;
        (*it)->worldTransform.transform_ = GetPositionByLength(addRotCtrlPoint_);
        (*it)->worldTransform.scale_ = { 0.3f };
        (*it)->number = (*selectRotIterator_)->number + 1;

        // 以降の番号をインクリメント
        for (auto i = std::next(it); i != editRotCtrlPoints_.end(); ++i)
        {
            (*i)->number++;
        }
    }
    else
    {// 選択していないとき
        editRotCtrlPoints_.emplace_back(std::make_unique<ControlPoint>());
        editRotCtrlPoints_.back()->posOnLine = addRotCtrlPoint_;
        editRotCtrlPoints_.back()->worldTransform.Initialize();
        editRotCtrlPoints_.back()->worldTransform.transform_ = GetPositionByLength(addRotCtrlPoint_);
        editRotCtrlPoints_.back()->worldTransform.scale_ = { 0.3f };
        editRotCtrlPoints_.back()->worldTransform.UpdateData();
        editRotCtrlPoints_.back()->number = static_cast<uint32_t>(editRotCtrlPoints_.size());
    }

    return;
}

void CatmulRomSpline::InsertPosCtrlPoint()
{
    if (selectPosIterator_ == editPosCtrlPoints_.end())
        return;

    // 並べ替えて
    SortPosCtrlPointByNum();
    // 同じ番豪の要素探して

    // insertNumを指すいてれーた
    auto insertIt = std::find_if(editPosCtrlPoints_.begin(), editPosCtrlPoints_.end(),
                                 [&](const std::unique_ptr<ControlPoint>& cp)
                                 {return cp->number == insertNumber_; });
    if (insertIt == editPosCtrlPoints_.end())
    {
        insertIt = std::prev(editPosCtrlPoints_.end());
        insertNumber_ = (*insertIt)->number;
    }

    uint32_t selecNum = (*selectPosIterator_)->number;

    if (selecNum > insertNumber_)
    {// 前に挿入するとき
        // insertの前に移動
        editPosCtrlPoints_.splice(insertIt, editPosCtrlPoints_, selectPosIterator_);

        /// 番号の更新
        // selectにinsertの番号を入れる
        (*selectPosIterator_)->number = insertNumber_;

        for (auto it = std::next(selectPosIterator_); it != editPosCtrlPoints_.end(); ++it)
        {// nextをインクリメント
            (*it)->number++;
            if ((*it)->number == selecNum)
                break;
        }
    }
    else
    {
        // insert後ろ(insertのnextの前)に入れる
        editPosCtrlPoints_.splice(std::next(insertIt), editPosCtrlPoints_, selectPosIterator_);

        // selectにinsertNumを入れる
        (*selectPosIterator_)->number = insertNumber_;

        // insertからselectNumまで要素番号をデクリメント
        for (auto it = insertIt; it != editPosCtrlPoints_.end(); --it)
        {
            (*it)->number--;
            if ((*it)->number == selecNum)
                break;
        }


    }
}

void CatmulRomSpline::InsertRotCtrlPoint()
{

    if (selectRotIterator_ == editRotCtrlPoints_.end())
        return;

    // 並べ替えて
    SortRotCtrlPointByPos();
    // 同じ番豪の要素探して

    // insertNumを指すいてれーた
    auto insertIt = std::find_if(editRotCtrlPoints_.begin(), editRotCtrlPoints_.end(),
                                 [&](const std::unique_ptr<ControlPoint>& cp)
                                 {return cp->number == insertNumber_; });
    if (insertIt == editRotCtrlPoints_.end())
    {
        insertIt = std::prev(editRotCtrlPoints_.end());
        insertNumber_ = (*insertIt)->number;
    }

    uint32_t selecNum = (*selectRotIterator_)->number;

    if (selecNum > insertNumber_)
    {// 前に挿入するとき
        // insertの前に移動
        editRotCtrlPoints_.splice(insertIt, editRotCtrlPoints_, selectRotIterator_);

        /// 番号の更新
        // selectにinsertの番号を入れる
        (*selectRotIterator_)->number = insertNumber_;

        for (auto it = std::next(selectRotIterator_); it != editRotCtrlPoints_.end(); ++it)
        {// nextをインクリメント
            (*it)->number++;
            if ((*it)->number == selecNum)
                break;
        }
    }
    else
    {
        // insert後ろ(insertのnextの前)に入れる
        editRotCtrlPoints_.splice(std::next(insertIt), editRotCtrlPoints_, selectRotIterator_);

        // selectにinsertNumを入れる
        (*selectRotIterator_)->number = insertNumber_;

        // insertからselectNumまで要素番号をデクリメント
        for (auto it = insertIt; it != editRotCtrlPoints_.end(); --it)
        {
            (*it)->number--;
            if ((*it)->number == selecNum)
                break;
        }


    }
}

void CatmulRomSpline::DeletePosCtrlPoint()
{
    if (selectPosIterator_ == editPosCtrlPoints_.end())
        return;

    uint32_t addNum = 0;
    for (auto it = std::next(selectPosIterator_); it != editPosCtrlPoints_.end(); ++it)
    {
        it->get()->number = (*selectPosIterator_)->number + addNum++;
    }
    editPosCtrlPoints_.erase(selectPosIterator_);
    selectPosIterator_ = editPosCtrlPoints_.end();

}

void CatmulRomSpline::DeleteRotCtrlPoint()
{
    if (selectRotIterator_ == editRotCtrlPoints_.end())
        return;

    uint32_t addNum = 0;
    for (auto it = std::next(selectRotIterator_); it != editRotCtrlPoints_.end(); ++it)
    {
        it->get()->number = (*selectRotIterator_)->number + addNum++;
    }
    editRotCtrlPoints_.erase(selectRotIterator_);
    selectRotIterator_ = editRotCtrlPoints_.end();
}

void CatmulRomSpline::SortPosCtrlPointByNum()
{
    editPosCtrlPoints_.sort([](const std::unique_ptr<ControlPoint>& a, const std::unique_ptr<ControlPoint>& b) { return a->number < b->number; });

    return;
}

void CatmulRomSpline::SortRotCtrlPointByPos()
{
    editRotCtrlPoints_.sort([](const std::unique_ptr<ControlPoint>& a, const std::unique_ptr<ControlPoint>& b) { return a->posOnLine < b->posOnLine; });

    return;
}

void CatmulRomSpline::CalculateCatmulRomSpline()
{
    lineDrawPoint_.clear();

    if (editPosCtrlPoints_.size() < 4)
        return;

    ConvertToFinalDataOfPosCtrl();

    GenerateDrawPoints();
    CalculateLengths();

    return;
}

void CatmulRomSpline::CalculatePositinByPosOnLine()
{
    if (editPosCtrlPoints_.size() < 4)
        return;

    Vector3 rotate = { 0.0f, 0.0f, 0.0f };

    posOnLine_ += speed_ * deltaTime_;
    if (posOnLine_ > totalLength_) {
        return;
        posOnLine_ = 0.0f;
    }

    float posT = GetPositionParameterForDistance(posOnLine_);

    Vector3 positionByT_ = CalculateCatmulRomPoint(posT);
    pMoveObj_->transform_ = positionByT_;

    ///*回転*///
    float t = GetRotateParameterForDistance(posOnLine_);

    pMoveObj_->rotate_ = (Rotate(t));

    moveObjTrans_.transform_ = pMoveObj_->transform_;
    moveObjTrans_.rotate_ = pMoveObj_->rotate_;
    moveObjTrans_.UpdateData();
}

Vector3 CatmulRomSpline::Rotate(float _t)
{
    ConvertToFinalDataOfRotCtrl();

    if (finalRotCtrlPoints_.empty())
    {
        return { 0,0,0 };
    }

    size_t divistion = finalRotCtrlPoints_.size() - 1;
    size_t index = static_cast<size_t>(std::floor(_t * divistion));
    index = std::clamp(index, size_t(0), finalRotCtrlPoints_.size() - 1);

    float t = _t * divistion - std::floor(_t * divistion);

    size_t index1 = index + 1;
    if (index1 > finalRotCtrlPoints_.size() - 1) {
        index1 = index;
    }

    Vector3 rotate1 = finalRotCtrlPoints_[index].rotate;
    Vector3 rotate2 = finalRotCtrlPoints_[index1].rotate;

    Vector3 result /*= Lerp(rotate1, rotate2, t)*/;

    result.x = LerpShortAngle(rotate1.x, rotate2.x, t);
    result.y = LerpShortAngle(rotate1.y, rotate2.y, t);
    result.z = LerpShortAngle(rotate1.z, rotate2.z, t);


    return result;
}

void CatmulRomSpline::SelectPoint(const Matrix4x4& _vp)
{
    Vector2 mPos = Input::GetInstance()->GetMousePosition();

    Matrix4x4 viewport = MakeViewportMatrix(0, 0, WinApp::kWindowWidth_, WinApp::kWindowHeight_, 0, 1);
    Matrix4x4 matVPVp = _vp * viewport;

    if (!selectRot_) {
        for (auto it = editPosCtrlPoints_.begin(); it != editPosCtrlPoints_.end(); ++it) {
            // スクリーン座標へ変換
            Vector3 pointPosition = Transform((*it)->worldTransform.GetWorldPosition(), matVPVp);
            Vector2 pointPositionOfscreen(pointPosition.x, pointPosition.y);

            // 範囲内にあったらイテレータを保存
            float distance = (mPos - pointPositionOfscreen).Length();
            if (distance <= hitRadius_) {
                // 同じものを選んでいたらキャンセル
                // そうじゃなければ選択
                if (selectPosIterator_ != editPosCtrlPoints_.end() && it == selectPosIterator_) {
                    selectPosIterator_ = editPosCtrlPoints_.end();
                    insertNumber_ = static_cast<uint16_t>(editPosCtrlPoints_.size());
                }
                else {
                    selectPosIterator_ = it;
                    selectRotIterator_ = editRotCtrlPoints_.end();
                    addPosCtrlPoint_ = (*it)->worldTransform.GetWorldPosition();
                    insertNumber_ = static_cast<uint16_t>(it->get()->number);
                }
            }
        }
    }
    else {
        for (auto it = editRotCtrlPoints_.begin(); it != editRotCtrlPoints_.end(); ++it) {
            Vector3 pointPosition = Transform((*it)->worldTransform.GetWorldPosition(), matVPVp);
            Vector2 pointPositionOfscreen(pointPosition.x, pointPosition.y);

            // 範囲内にあったらイテレータを保存
            float distance = (mPos - pointPositionOfscreen).Length();
            if (distance <= hitRadius_) {
                if (selectRotIterator_ != editRotCtrlPoints_.end() && it == selectRotIterator_) {
                    selectRotIterator_ = editRotCtrlPoints_.end();
                    insertNumber_ = static_cast<uint16_t>(editRotCtrlPoints_.size());
                }
                else {
                    selectRotIterator_ = it;
                    selectPosIterator_ = editPosCtrlPoints_.end();
                    addRotCtrlPoint_ = (*it)->posOnLine;
                    insertNumber_ = static_cast<uint16_t>(it->get()->number);
                }
            }
        }
    }

}

void CatmulRomSpline::RegisterDrawPoint()
{
    auto instance = LineDrawer::GetInstance();

    for (size_t index = 1; index < lineDrawPoint_.size(); index++)
    {
        instance->RegisterPoint(lineDrawPoint_[index - 1], lineDrawPoint_[index]);
    }
}

void CatmulRomSpline::ImGui()
{
#ifdef _DEBUG

    ImGui::Begin("CatmulRom");
    ImGui::Checkbox("drawCtrlPoint", &isDrawCtrlPoint_);
    if (isDrawCtrlPoint_)
    {
        ImGui::Checkbox("drawPosPoint", &isDrawPosCtrlPoint_);
        ImGui::Checkbox("drawRotPoint", &isDrawRotCtrlPoint_);
    }
    ImGui::Checkbox("drawMoveObj", &drawMoveObj_);
    if (ImGui::Checkbox("Move", &isMove_)) {
        isDrawCtrlPoint_ = !isMove_;
        isDrawRotCtrlPoint_ = !isMove_;
    }
    ImGui::DragFloat("selectRadius", &hitRadius_, 0.1f);

    ImGui::BeginTabBar("point");
    if (ImGui::BeginTabItem("positoin"))
    {
        selectRot_ = false;
        ImGui::DragFloat3("AddPosition", &addPosCtrlPoint_.x, 0.01f);

        if (ImGui::Button("AddControlPoint") /*|| Input::GetInstance()->PushKey(DIK_LALT) && Input::GetInstance()->IsTriggerMouse(0)*/)
        {
            AddPosCtrlPoint();
            isChangeCtrlPoint_ = true;
        }
        if (ImGui::Button("DeleteControlPoint"))
        {
            DeletePosCtrlPoint();
            isChangeCtrlPoint_ = true;
        }
        ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("rotation"))
    {
        selectRot_ = true;
        ImGui::SliderFloat("AddPosition", &addRotCtrlPoint_, 0.0f, totalLength_ - 0.01f);

        if (ImGui::Button("AddControlPoint") /*|| Input::GetInstance()->PushKey(DIK_LALT) && Input::GetInstance()->IsTriggerMouse(0)*/)
        {
            AddRotCtrlPoint();
            isChangeCtrlPoint_ = true;
        }
        if (ImGui::Button("DeleteControlPoint"))
        {
            DeleteRotCtrlPoint();
            isChangeCtrlPoint_ = true;
        }
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();

    if (selectPosIterator_ != editPosCtrlPoints_.end()) {
        if (ImGui::DragFloat3("positoin", &(selectPosIterator_->get()->worldTransform.transform_.x), 0.01f))
        {
            isChangeCtrlPoint_ = true;
        }
        ImGui::Text("number : %d", (int)selectPosIterator_->get()->number);

        ImGui::Spacing();
        if (ImGui::Button("Insert")) {
            InsertPosCtrlPoint();
            isChangeCtrlPoint_ = true;
        }
        ImGui::SameLine();
        int tempNum = static_cast<int>(insertNumber_);
        ImGui::SetNextItemWidth(100.0f);
        ImGui::InputInt("Number", &tempNum);
        insertNumber_ = static_cast<uint32_t>(tempNum);
    }
    if (selectRotIterator_ != editRotCtrlPoints_.end()) {
        if (ImGui::SliderFloat("positoin", &(selectRotIterator_->get()->posOnLine), 0.01f, totalLength_ - 0.01f)) {
            isChangeCtrlPoint_ = true;
        }
        if (ImGui::DragFloat3("rotate", &selectRotIterator_->get()->worldTransform.rotate_.x, 0.01f)) {
            isChangeCtrlPoint_ = true;
        }
        ImGui::Text("number : %d", (int)selectRotIterator_->get()->number);
    }

    if (ImGui::SliderFloat("positionOnLine", &posOnLine_, 0.0f, totalLength_))
        CalculatePositinByPosOnLine();
    ImGui::DragFloat("speed", &speed_, 0.1f);

    /*for (float l : areaLength_) {
        ImGui::Text("%f", l);
    }*/

    ImGui::End();
#endif // _DEBUG
}