#include "ObjectManager.h"
#include "DxLib.h"

// シングルトン取得
ObjectManager& ObjectManager::GetInstance()
{
	static ObjectManager instance;
	return instance;
}

// 全破棄の実装
void ObjectManager::ClearAll()
{
	m_pool.ClearAll();
}

// デバッグ表示: 現在のアクティブオブジェクト数を画面に描画する
void ObjectManager::DrawObjectCount(int x, int y) const
{
	size_t active = m_pool.ActiveCount();
	size_t total = m_pool.TotalCreated();
	size_t deleted = m_pool.TotalDeleted();
	// 例: "Objects: 7 (Created: 42 Deleted: 35)"
	DrawFormatString(x, y, GetColor(255, 255, 255),
		"現オブジェクト数: %d (総生産数: %d 削除数: %d)",
		static_cast<int>(active), static_cast<int>(total), static_cast<int>(deleted));
}

void ObjectManager::UpdateAllObjectsScene(const std::weak_ptr<SceneBase>& scene)
{
	m_pool.UpdateAllObjectsScene(scene);
}

// 現在のシーンを登録してプール内オブジェクトの scene を更新するユーティリティ
void ObjectManager::RegisterCurrentScene(const std::shared_ptr<SceneBase>& scene)
{
	SceneBase::SetCurrentScene(scene);
	m_pool.UpdateAllObjectsScene(SceneBase::GetCurrentSceneWeak());
}