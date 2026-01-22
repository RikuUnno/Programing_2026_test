#include "BulletTrigger.h"
#include "ObjectManager.h"
#include "Bullet.h"
#include "Time.h"
#include <cmath>

ObjectHandle BulletTrigger::Shoot(const VECTOR& localOffset, const VECTOR& direction)
{
	const float defaultBulletRadius = 8.0f;

	// ownerObjectが有効か確認し、シーンを取得する
	std::shared_ptr<GameObject> ownerObj = m_ownerObject.lock();
	if (!ownerObj) {
		// オーナーが存在しない場合は弾を撃てない（シーンが不定のため）
		return ObjectHandle(); 
	}
	
	// オーナーの所属シーン（ColliderManagerの判定で重要）
	std::weak_ptr<SceneBase> scene = ownerObj->GetSceneWeak();

	// owner が居なければ localOffset をワールドとみなす
	VECTOR spawnPos = localOffset;
	VECTOR worldDir = direction;

	// Transform計算 (m_ownerTransformの更新は行われている前提だが、ownerObj->GetTransform()を使うほうが安全かも)
	std::shared_ptr<Transform> ownerT = ownerObj->GetTransform(); // m_ownerTransform.lock();
	if (ownerT) {
		// 親のワールド位置
		VECTOR ownerWorld = ownerT->MatrixToVector();

		// 親のスケール（ローカル offset を拡縮）
		VECTOR s = ownerT->GetScale();
		VECTOR scaledOffset = VGet(localOffset.x * s.x, localOffset.y * s.y, localOffset.z * s.z);

		// 親の回転（Z 回転を想定）で offset を回す（度->ラジアン）
		const float PI = 3.14159265f;
		float rad = ownerT->GetRotation().z * (PI / 180.0f);
		float rx = std::cos(rad) * scaledOffset.x - std::sin(rad) * scaledOffset.y;
		float ry = std::sin(rad) * scaledOffset.x + std::cos(rad) * scaledOffset.y;

		// 親基準の localOffset -> ワールド座標
		spawnPos.x = ownerWorld.x + rx;
		spawnPos.y = ownerWorld.y + ry;
		spawnPos.z = ownerWorld.z + scaledOffset.z;

		// direction もローカル指定なら回転してワールド方向に
		float dx = direction.x, dy = direction.y;
		float dRx = std::cos(rad) * dx - std::sin(rad) * dy;
		float dRy = std::sin(rad) * dx + std::cos(rad) * dy;
		worldDir = VGet(dRx, dRy, 0.0f);
	}

	// 前方に少し押し出して親や子と被らないようにする（margin）
	float len = std::sqrt(worldDir.x*worldDir.x + worldDir.y*worldDir.y);
	if (len <= 1e-6f) {
		// 無効な方向は上方向へ
		worldDir = VGet(0.0f, -1.0f, 0.0f);
		len = 1.0f;
	}
	VECTOR push = VGet((worldDir.x/len) * (defaultBulletRadius + 4.0f),
	                   (worldDir.y/len) * (defaultBulletRadius + 4.0f),
	                   0.0f);
	spawnPos.x += push.x;
	spawnPos.y += push.y;

	// デバッグ出力
	DebugLogFmt("[Shoot] spawn=(%.2f,%.2f) dir=(%.2f,%.2f)\n", spawnPos.x, spawnPos.y, worldDir.x, worldDir.y);

	// 生成（ワールド位置を直接渡す。親は付けない）
	// 取得した scene を渡す
	auto h = ObjectManager::GetInstance().Create<Bullet>(scene, spawnPos, defaultBulletRadius);
	if (!h.IsValid()) return h;

	// 生成直後に shared_ptr を取り出してパラメータを設定
	if (auto b = ObjectManager::GetInstance().Get<Bullet>(h)) {
		// worldDir を正規化して渡す
		b->SetDirection(worldDir);
		b->SetSelfHandle(h);

		// Transform を生成位置に確実に反映しておく（コンストラクタで設定済みのはずだが確実性のため）
		b->GetTransform()->SetPosition(spawnPos);
		b->GetTransform()->UpdateMatrix();
		b->GetTransform()->LocalToWorldMatrix();

		// ObjectGroup にハンドルのみ登録（所有はしない）
		m_bullets.Add(h);
	}
	return h;
}

void BulletTrigger::Update()
{
	// 弾の Update を一括で呼ぶ
	m_bullets.UpdateAll();

}

void BulletTrigger::Draw()
{
	// 弾の Draw を一括で呼ぶ
	m_bullets.DrawAll();
}

void BulletTrigger::SetOwnerObject(const std::weak_ptr<GameObject>& owner)
{
	m_ownerObject = owner; 
	SetOwnerTransform();	// Transform も更新
}

void BulletTrigger::SetOwnerTransform()
{
	if (auto owner = m_ownerObject.lock()) {
		m_ownerTransform = owner->GetTransform();
	}
	else {
		m_ownerTransform.reset();
	}
}