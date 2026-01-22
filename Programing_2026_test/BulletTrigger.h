// Bulletを撃つ為の一種のマネージャー
#pragma once
#include "ObjectGroup.h"
#include "ObjectHandle.h"
#include "Transform.h"
#include <memory>

class BulletTrigger
{
private:
	ObjectGroup m_bullets;                      // 弾ハンドルの管理（所有しない）
	std::weak_ptr<GameObject> m_ownerObject;    // 親の GameObject（所有しない）
	std::weak_ptr<Transform> m_ownerTransform;  // 親の Transform（所有しない）


public:
	BulletTrigger() = default;
	~BulletTrigger() = default;

	// 親の GameObject を設定（所有はしない）
	void SetOwnerObject(const std::weak_ptr<GameObject>& owner);

private:
	// 親の Transform を設定（所有はしない）
	void SetOwnerTransform();

public:

	// 単発発射（必要なパラを渡す）
	ObjectHandle Shoot(const VECTOR& localOffset, const VECTOR& direction);

	// Update を呼んで弾を更新する（Scene/Character の Update から呼ぶ）
	void Update();
	// Draw を呼んで弾を描画する（Scene/Character の Draw から呼ぶ）
	void Draw();

	// シーン／キャラ終了時に呼ぶ
	void EndAll() { m_bullets.EndAll(); }
	void Clear()  { m_bullets.Clear(); }
};