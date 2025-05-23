#pragma once

#include <Math/Vector/Vector2.h>
#include <Math/Vector/Vector3.h>

#include <Features/Collision/Collider/Collider.h>

#include <cstdint>
#include <list>
#include <vector>
#include <memory>
#include <iterator>


class Cell;

class ObjectForTree
{
public:

    ObjectForTree() = default;
    ~ObjectForTree() = default;

    /// <summary>
    /// データを設定
    /// </summary>
    void SetData(Collider* _data) { data_ = _data; } // データ設定

    /// <summary>
    /// データを取得
    /// </summary>
    Collider* GetData() { return data_; } // データ取得

    /// <summary>
    /// セルを登録
    /// </summary>
    void SetCell(Cell* _cell) { belongingCell_ = _cell; } // 所属する空間を設定

    /// <summary>
    /// セルを取得
    /// </summary>
    Cell* GetCell() { return belongingCell_; } // 所属する空間を取得

    /// <summary>
    /// 空間に登録されているデータのイテレータを取得
    /// </summary>
    /// <param name="_iter"> 空間に登録されているデータのイテレータ </param>
    void SetDataIter(std::list<std::shared_ptr<ObjectForTree>>::iterator _iter) { dataIter_ = _iter; } // 空間に登録されているデータのイテレータを設定

    std::list<std::shared_ptr<ObjectForTree>>::iterator& GetDataIter() { return dataIter_; }


    /// <summary>
    /// 前のデータを取得
    /// </summary>
    /// <returns> 前のデータ </returns>
    std::shared_ptr<ObjectForTree> GetNextData(); // 次のデータを取得

    void Remove();


private:
    Collider* data_ = nullptr; // データ

    Cell* belongingCell_ = nullptr; // 所属する空間
    std::list<std::shared_ptr<ObjectForTree>>::iterator dataIter_; // 空間に登録されているデータのイテレータ 前後データアクセス用itel

};

/// <summary>
/// ４分木空間分割の空間
/// </summary>
class Cell
{
public:
    Cell() = default;

    ~Cell();

    void Reset();

    std::list<std::shared_ptr<ObjectForTree >>& GetData() { return data_; } // データ取得

    bool RegisterData(std::shared_ptr<ObjectForTree > _data);

    bool RemoveDataByIter(typename std::list<std::shared_ptr<ObjectForTree >>::iterator _iter);

    // データを指定して削除 - 内部でイテレータを使用
    bool RemoveData(std::shared_ptr<ObjectForTree > _data);

    std::shared_ptr<ObjectForTree > GetFirstData();

    // オブジェクトからの削除通知
    bool OnRemove(std::shared_ptr<ObjectForTree > _removeObj);

private:
    std::list<std::shared_ptr<ObjectForTree >> data_;
};
