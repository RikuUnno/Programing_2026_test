#include "GameObject.h"
#include <memory>

GameObject::GameObject()
	: m_scene()
{}

GameObject::GameObject(const std::weak_ptr<SceneBase> scene)
	: m_scene(scene)
{}

GameObject::~GameObject()
{}

// 子を追加すると Transform の親も設定する
void GameObject::AddChild(std::shared_ptr<GameObject> child)
{
	// nullptr チェック
	if(child == nullptr) return;

	// 既に同じ子が存在するかチェック（shared_ptr 同士の比較は中身のポインタを比較する）
	if (std::find(m_child.begin(), m_child.end(), child) != m_child.end()) return;

	m_child.push_back(child);
	child->GetTransform()->SetParent(m_transform);
}

// 子を削除する
void GameObject::RemoveChild(std::shared_ptr<GameObject> child)
{
	auto it = std::find(m_child.begin(), m_child.end(), child); // イテレータを取得
	if (it != m_child.end()) // 見つかったら削除
	{
		child->GetTransform()->ClearParent(); // 親設定をクリア
		m_child.erase(it);					  // リストから削除
	}
}

// 全ての子を削除する
void GameObject::RemoveAllChild()
{
	for (auto& child : m_child)
	{
		child->GetTransform()->ClearParent(); // 親設定をクリア
	}
	m_child.clear(); // リストをクリア
}

// 生成直後に呼ぶ初期化（デフォルトは空実装）
void GameObject::InitObject() {}

// 派生クラスが上書きする Start（最初に一回だけ呼ぶ用途）
void GameObject::Start() {}

// 毎フレーム呼ばれる（派生で実装）
void GameObject::Update() {}

// 描画用（派生で実装）
void GameObject::Draw() {}

// シーンから削除されるときに一回だけ呼ばれる
void GameObject::End() {}

