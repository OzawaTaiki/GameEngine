#include "Cell.h"

std::shared_ptr<ObjectForTree> ObjectForTree::GetNextData()

{
    if (std::next(dataIter_) == belongingCell_->GetData().end())
    {
        return nullptr;
    }
    else
    {
        return *(std::next(dataIter_));
    }
}

void ObjectForTree::Remove()
    {
        if (belongingCell_)
        {
            belongingCell_->RemoveDataByIter(dataIter_); // ��Ԃ���폜
            belongingCell_ = nullptr; // ���������Ԃ�null�ɐݒ�
            //dataIter_ = nullptr;
        }
    }

Cell::~Cell()
{
    for (auto& data : data_)
    {
        if (data)
        {
            data->SetCell(nullptr); // ���������Ԃ�null�ɐݒ�
        }
    }
    data_.clear();
}

bool Cell::RegisterData(std::shared_ptr<ObjectForTree> _data)
{
    if (!_data) return false;
    if (_data->GetCell() == this) return false;

    data_.push_front(_data);
    _data->SetCell(this);
    _data->SetDataIter(data_.begin()); // �C�e���[�^�ݒ�
    return true;
}

bool Cell::RemoveDataByIter(typename std::list<std::shared_ptr<ObjectForTree>>::iterator _iter)
{
    if (_iter == data_.end()) return false;

    auto data = *_iter;
    if (data) {
        data->SetCell(nullptr);
    }

    data_.erase(_iter); // O(1)�̍폜����
    return true;
}

bool Cell::RemoveData(std::shared_ptr<ObjectForTree> _data)
{
    if (!_data || _data->GetCell() != this) return false;

    // �f�[�^�����C�e���[�^���g�p���Č����I�ɍ폜
    auto iter = _data->GetDataIter();
    _data->SetCell(nullptr);
    data_.erase(iter); // O(1)�̍폜����

    return true;
}

std::shared_ptr<ObjectForTree> Cell::GetFirstData()
{
    if (data_.empty()) return nullptr;
    return data_.front();
}

bool Cell::OnRemove(std::shared_ptr<ObjectForTree> _removeObj)
{
    // �폜���\�b�h�̓���
    return RemoveData(_removeObj);
}
