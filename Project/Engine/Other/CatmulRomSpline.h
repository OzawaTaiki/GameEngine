#pragma once

#include "Model.h"
#include "WorldTransform.h"
#include "JsonLoader.h"
#include "ObjectColor.h"

#include <Vector3.h>


#include <list>
#include <string>
#include <vector>

class Camera;
class CatmulRomSpline
{
private:
    /*--------構造体たち---------*/
    // 回転制御点
    struct RotatePoint
    {
        Vector3			rotate;			// 三次元
        float			posOnLine;		// ライン上の位置
    };
    // 座標，回転兼用
    struct ControlPoint
    {
        WorldTransform	worldTransform;	// transform
        float			posOnLine;		// ライン上の位置
        uint32_t		number;			// 計算用ナンバー
    };


public:

    CatmulRomSpline();
    ~CatmulRomSpline();


    /// <summary>
    /// 移動オブジェクトのポインタをセット
    /// <summary>
    void SetMoveObjTrans(WorldTransform* _prt) { pMoveObj_ = _prt; }

    void Initialize(const std::string& _filePath);
    void Update(const Matrix4x4& _vp);
    void Draw(const Camera* _camera);

    bool IsMove()const { return isMove_; }
    void Move() { isMove_ = true; }

private:
    /// <summary>
    /// finalRotDataに変換
    /// </summary>
    /// <param name="_rotArr"> 回転成分</param>
    /// <param name="_posArr"> ライン上の位置 </param>
    void CreateFinalRotData(const std::vector<Vector3>& _rotArr, const std::vector<float>& _posArr);

    /// <summary>
    /// 編集用データへ変換
    /// </summary>
    void ConvertToEditableData();

    /// <summary>
    /// 計算用データへ変換
    /// </summary>
    void ConvertToFinalDataOfPosCtrl();

    /// <summary>
    /// 計算用データへ変換
    /// </summary>
    void ConvertToFinalDataOfRotCtrl();

    /// <summary>
    /// ライン上の座標を取得する
    /// </summary>
    /// <param name="_length">ライン上の位置</param>
    /// <returns>ライン上の座標</returns>
    Vector3 GetPositionByLength(float _length);

    /// <summary>
    /// lengthから媒介変数tを得る
    /// </summary>
    /// <param name="_length">ライン上の位置</param>
    /// <returns>ライン上の位置 t</returns>
    float GetPositionParameterForDistance(float _length);

    /// <summary>
    /// targetDisから媒介変数tを得る
    /// </summary>
    /// <param name="_targetDis">ライン上の位置</param>
    /// <returns>媒介変数</returns>
    float GetRotateParameterForDistance(float _targetDis);

    /// <summary>
    /// tに基づいてライン上の点を計算する
    /// </summary>
    /// <param name="_t">媒介変数</param>
    /// <returns>ライン上の座標 </returns>
    Vector3 CalculateCatmulRomPoint(float _t);

    /// <summary>
    /// ライン上の座標を計算する
    /// </summary>
    /// <param name="_index0">0番目のindex</param>
    /// <param name="_index1">1番目のindex</param>
    /// <param name="_index2">2番目のindex</param>
    /// <param name="_index3">3番目のindex</param>
    /// <param name="_t">媒介変数</param>
    /// <returns>ライン上の座標</returns>
    Vector3 CalculateCatmulRomAreaLine(size_t _index0, size_t _index1, size_t _index2, size_t _index3, float _t);

    /// <summary>
    /// 描画点の生成
    /// </summary>
    void GenerateDrawPoints();

    /// <summary>
    /// 区間距離の計算
    /// </summary>
    void CalculateLengths();

    /// <summary>
    /// 座標制御点を追加
    /// </summary>
    void AddPosCtrlPoint();

    /// <summary>
    /// 回転制御点を追加
    /// </summary>
    void AddRotCtrlPoint();

    /// <summary>
    /// 座標制御点の挿入
    /// </summary>
    void InsertPosCtrlPoint();

    /// <summary>
    /// 回転制御点の挿入
    /// </summary>
    void InsertRotCtrlPoint();

    /// <summary>
    /// 座標制御点の削除
    /// </summary>
    void DeletePosCtrlPoint();

    /// <summary>
    /// 回転制御点の削除
    /// </summary>
    void DeleteRotCtrlPoint();

    /// <summary>
    /// 座標制御点を番号順に並び替え
    /// </summary>
    void SortPosCtrlPointByNum();

    /// <summary>
    /// 回転制御点を位置順に並び替え
    /// </summary>
    void SortRotCtrlPointByPos();

    /// <summary>
    /// スプライン曲線を計算
    /// </summary>
    void CalculateCatmulRomSpline();

    /// <summary>
    /// posOnLineから座標を計算しカメラへセット
    /// </summary>
    void CalculatePositinByPosOnLine();

    /// <summary>
    /// 回転の処理
    /// </summary>
    Vector3 Rotate(float _t);

    void SelectPoint(const Matrix4x4& _vp);

    void RegisterDrawPoint();

    WorldTransform* pMoveObj_ = nullptr;      // 移動オブジェクトのポインタ
    WorldTransform							moveObjTrans_ = {};
    ObjectColor* moveObjColor_ = nullptr;

    JsonLoader* jsonLoader_ = nullptr;		// jsonLoaderのポインタ
    Model* posModel_ = nullptr;		// 座標制御点モデル
    uint32_t								posModelTexture_ = 0;
    Model* rotModel_ = nullptr;		// 回転制御点モデル
    uint32_t								rotModelTexture_ = 0;
    ObjectColor* colorWhite_ = nullptr;
    ObjectColor* colorRed_ = nullptr;
    std::string								filepath_ = {};			// 読み書き用ファイル

    std::vector<float>						areaLength_ = {};			// 区間ごとの距離
    std::vector<float>						cumulativeLength_ = {};			// 区間ごと累積距離
    std::vector<Vector3>					lineDrawPoint_ = {};			// ライン描画用配列

    std::list<std::unique_ptr<ControlPoint>>::iterator	selectPosIterator_ = {};			// 選択中の制御点のイテレータ
    std::list<std::unique_ptr<ControlPoint>>::iterator	selectRotIterator_ = {};			// 選択中の制御点のイテレータ
    std::list<std::unique_ptr<ControlPoint>>			editPosCtrlPoints_ = {};			// 確定前の座標制御点
    std::list<std::unique_ptr<ControlPoint>>			editRotCtrlPoints_ = {};			// 確定前の回転制御点
    std::vector<Vector3>								finalPosCtrlPoints_ = {};			// 確定した座標制御点
    std::vector<RotatePoint>							finalRotCtrlPoints_ = {};			// 確定した回転制御点

    Vector3									addPosCtrlPoint_ = {};			// 座標制御点 追加座標
    float									addRotCtrlPoint_ = {};			// 回転制御点 追加位置

    uint32_t								insertNumber_ = 0;			// 挿入インデックス
    uint32_t								lineSegmentCount_ = 16;			// ラインの分割数
    float									totalLength_ = 0.0f;			// 合計の長さ
    float									hitRadius_ = 0.0f;			// マウスとの判定用
    float									posOnLine_ = 0.0f;			// ライン上の位置
    float									speed_ = 0.0f;			// 移動スピード
    float									deltaTime_ = 0.0f;			// デルタタイム

    bool									isDrawCtrlPoint_ = false;		// 制御点の描画フラグ
    bool									isDrawPosCtrlPoint_ = false;		// 制御点の描画フラグ
    bool									isDrawRotCtrlPoint_ = false;		// 制御点の描画フラグ
    bool									isMove_ = false;		// 移動フラグ
    bool									drawMoveObj_ = false;		// 移動objの描画フラグ
    bool									selectRot_ = false;		// 回転制御点を選択しているか
    bool									isChangeCtrlPoint_ = false;		// 編集したか否か

    void ImGui();


};