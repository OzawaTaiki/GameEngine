#include <Features/Json/JsonBinder.h>

// 要素型を取得するための基本テンプレート
template <typename T>
struct element_type {
    using type = void; // デフォルトで無効な型
};

// 特定の型に対して部分特殊化を適用
template <typename U>
struct element_type<RefVector<U>> {
    using type = U;
};

template <typename U>
struct element_type<std::list<U>> {
    using type = U;
};

// ヘルパーエイリアス
template <typename T>
using element_type_t = typename element_type<T>::type;

JsonBinder::JsonBinder(const std::string& _name, const std::string& _folderPath)
{
    groupName_ = _name;
    folderPath_ = _folderPath;
    jsonHub_ = JsonHub::GetInstance();
}

void JsonBinder::SetFolderPath()
{
    jsonHub_->SetDirectoryPathFromRoot(folderPath_);
}

void JsonBinder::Save()
{
    RegisterVariable();

    jsonHub_->SetDirectoryPathFromRoot(folderPath_);
    jsonHub_->Save(groupName_);

}
void JsonBinder::RegisterVariable()
{
    for (auto& [name, pVal] : memberPtrMap_)
    {
        std::visit([&](auto&& _arg)
                   {
                       using T = std::decay_t<decltype(_arg)>;
                           if constexpr(std::is_pointer_v<T>)
                           {
                               SendVariable(name, *std::get<T>(pVal.address));
                           }
                           else if constexpr (std::is_same_v<T, RefVector<int32_t>> ||
                                              std::is_same_v<T, RefVector<uint32_t>> ||
                                              std::is_same_v<T, RefVector<float>> ||
                                              std::is_same_v<T, RefVector<Vector2>> ||
                                              std::is_same_v<T, RefVector<Vector3>> ||
                                              std::is_same_v<T, RefVector<Vector4>> ||
                                              std::is_same_v<T, RefVector<std::string>>)
                           {
                               std::vector<element_type_t<T>> vec;
                               vec = std::get<RefVector<element_type_t<T>>>(pVal.address).get();
                               SendVariable(name, vec);
                           }
                           else if constexpr (std::is_same_v<T, std::list<int32_t>*> ||
                                              std::is_same_v<T, std::list<uint32_t>*> ||
                                              std::is_same_v<T, std::list<float>*> ||
                                              std::is_same_v<T, std::list<Vector2>*> ||
                                              std::is_same_v<T, std::list<Vector3>*> ||
                                              std::is_same_v<T, std::list<Vector4>*> ||
                                              std::is_same_v<T, std::list<std::string>*>)
                           {
                               std::list<element_type_t<T>> list;
                               list = std::get<std::list<element_type_t<T>>>(pVal.address).get();
                               RegisterVariable(name, list);
                           }
                   }, pVal.address);

    }
}
