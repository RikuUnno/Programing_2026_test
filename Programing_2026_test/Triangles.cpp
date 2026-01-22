#include "Triangles.h"
#include "ChildTriangles.h"
#include "Info.h"
#include "Time.h"
#include "ObjectManager.h"
#include "Bullet.h"
#include "ColliderInfo.h" // Layer定義

// コンストラクタ
Triangles::Triangles(const std::weak_ptr<SceneBase> scene, VECTOR position, float objectRadiusSize)
	: TriangleBase(scene)
	, m_objectRadiusSize(objectRadiusSize)
{
	m_transform->SetPosition(position); // 初期値を設定
	m_transform->SetRotation({ 0.0f, 0.0f, 0.0f });
	m_transform->SetScale({ 1.0f, 1.0f, 1.0f });

	// Child 用変数初期化
	m_childObjectRadius = 20.0f;
	m_childOffsetSize = 100.0f;

	// 初期三角形サイズ（スケール考慮は Update 側で再計算する）
	TriangleBaseInitUsingTransform(m_objectRadiusSize, 90.0f);

}

// デストラクタ
Triangles::~Triangles()
{
	// Release 管理は End() で行うが念のため
	m_ChildObjectGroup.Clear();
	m_bulletTrigger.Clear();
}

// 生成直後(Factory)に呼ぶ初期化
void Triangles::InitObject()
{
	// 再利用時の初期化などがあればここに
	TriangleBaseInitUsingTransform(m_objectRadiusSize);

	// コライダー初期化
	m_collider.SetOwner(shared_from_this());
	m_collider.SetupFromBase(m_objectRadiusSize, 180.0f); // 下向き
	
	// レイヤー設定 (Enemy)
	m_collider.SetLayer(Layer::Enemy);
	
	// マスク設定: PlayerBullet と Player に当たりたい
	m_collider.SetMask((1 << Layer::PlayerBullet) | (1 << Layer::Player));

	// コールバック登録 (Enter)
	m_collider.SetOnCollisionEnterCallback([this](Collider* other) {
		this->OnHit(other);
	});
}

// シーン登録時に一回だけ呼ばれる
void Triangles::Start()
{
	m_isActive = true; // アクティブ化
	m_collider.SetActive(true); // コライダー有効化

	m_life = 300.0f; // 体力リセット

	// 子の生成（Start で行う)
	auto child_h_1 = ObjectManager::GetInstance().Create<ChildTriangles>(m_scene, VGet(m_childOffsetSize, 0, 0), m_childObjectRadius);
	auto child_h_2 = ObjectManager::GetInstance().Create<ChildTriangles>(m_scene, VGet(-m_childOffsetSize, 0, 0), m_childObjectRadius);
	auto child_h_3 = ObjectManager::GetInstance().Create<ChildTriangles>(m_scene, VGet(0, m_childOffsetSize, 0), m_childObjectRadius);
	auto child_h_4 = ObjectManager::GetInstance().Create<ChildTriangles>(m_scene, VGet(0, -m_childOffsetSize, 0), m_childObjectRadius);

	auto child1 = ObjectManager::GetInstance().Get<ChildTriangles>(child_h_1);
	auto child2 = ObjectManager::GetInstance().Get<ChildTriangles>(child_h_2);
	auto child3 = ObjectManager::GetInstance().Get<ChildTriangles>(child_h_3);
	auto child4 = ObjectManager::GetInstance().Get<ChildTriangles>(child_h_4);

	// 子は ObjectGroup 管理、Transform の親付けのみ行う（所有はしない）
	if (child1) { child1->GetTransform()->SetParent(m_transform); m_ChildObjectGroup.Add(child_h_1); }
	if (child2) { child2->GetTransform()->SetParent(m_transform); m_ChildObjectGroup.Add(child_h_2); }
	if (child3) { child3->GetTransform()->SetParent(m_transform); m_ChildObjectGroup.Add(child_h_3); }
	if (child4) { child4->GetTransform()->SetParent(m_transform); m_ChildObjectGroup.Add(child_h_4); }

	// BulletTrigger の所有 Transform を保証（ここでも設定可能)
	m_bulletTrigger.SetOwnerObject(shared_from_this());

}

// 毎フレーム呼ばれる
void Triangles::Update()
{
	if (m_isActive == false) return; // 非アクティブ時は処理しない

	// 体力チェック
	if (m_life <= 0.0f) {
		IsActive(false); // アクティブを無効化
		m_collider.SetActive(false); // コライダー無効化
		return;
	}

	// 移動
	Move();

	// 三角形の表示更新（Transform とスケールを考慮）
	TriangleBaseInitUsingTransform(m_objectRadiusSize, 90.0f);

	// 子の更新
	m_ChildObjectGroup.UpdateAll();

	// 弾の更新（内部的に ObjectGroup を更新）
	m_bulletTrigger.Update();

	// 発射タイマー処理
	float dt = static_cast<float>(Time::GetInstance().DeltaTime());
	m_fireTimer += dt;
	if (m_fireTimer >= m_fireRate) {
		Attack();
		m_fireTimer = 0.0f;
	}
}

// 毎フレーム描画
void Triangles::Draw()
{
	if (m_isActive == false) return; // 非アクティブ時は処理しない

	DrawStoredTriangleEdges(GetColor(255, 0, 0), GetColor(0, 255, 0), GetColor(0, 0, 255));
	m_ChildObjectGroup.DrawAll();
	m_bulletTrigger.Draw();
}

// シーンから削除されるときに一回だけ呼ばれる
void Triangles::End()
{
	// 子の親子関係解除・終了・解放
	m_ChildObjectGroup.RemoveAllChild();
	m_ChildObjectGroup.EndAll();
	m_ChildObjectGroup.Clear();

	// 弾の終了処理（発射機器に委譲）
	m_bulletTrigger.EndAll();
	m_bulletTrigger.Clear();

	// コライダー無効化
	m_collider.SetActive(false);
}

// 移動処理
void Triangles::Move()
{
	float dt = static_cast<float>(Time::GetInstance().DeltaTime());

	// 現在位置取得
	VECTOR pos = m_transform->GetPosition();

	// X 方向移動
	pos.x += static_cast<float>(m_moveDir) * static_cast<float>(m_moveSpeed) * dt;

	// 端で反転（半径分マージン）
	const float leftBound = m_objectRadiusSize;
	const float rightBound = static_cast<float>(Info::WINDOW_WIDTH) - m_objectRadiusSize;

	if (pos.x <= leftBound) {
		pos.x = leftBound;
		m_moveDir = 1;
	}
	else if (pos.x >= rightBound) {
		pos.x = rightBound;
		m_moveDir = -1;
	}

	// 反映
	m_transform->SetPosition(pos);
	m_transform->UpdateMatrix();
	m_transform->LocalToWorldMatrix();
}

// 攻撃処理（単純に真下に撃つ例。localOffset=0: 親の中心位置から発射)
void Triangles::Attack()
{
	// 発射方向はローカルで指定（下方向）
	VECTOR dirLocal = VGet(0.0f, 1.0f, 0.0f);

	// 子オフセット群と被らない発射位置を計算
	float maxChildOffset = m_childOffsetSize; // 子が配置されている最大オフセット（必要なら走査で算出）
	float margin = 4.0f;
	VECTOR offsetLocal = VGet(0.0f, m_objectRadiusSize + m_bulletRadius + margin + maxChildOffset * 0.0f, 0.0f);
	// 上：maxChildOffset*0.0f としているが、必要なら maxChildOffset を加える

	auto h = m_bulletTrigger.Shoot(offsetLocal, dirLocal);
	if (!h.IsValid()) return;

	if (auto b = ObjectManager::GetInstance().Get<Bullet>(h)) {
		b->SetSpeed(m_bulletSpeed);
		b->SetLifetime(m_bulletLife);
		b->SetBaseRadius(m_bulletRadius);
		b->SetColor(m_bulletColor);
		
		// Layer / Mask 設定
		b->SetLayer(Layer::EnemyBullet);	// 敵弾
		b->SetMask(1 << Layer::Player);		// プレイヤーに当たる
	}
}

// 衝突処理
void Triangles::OnHit(Collider* other)
{
	m_life -= 10.0f; // 10ダメージ
	// プレイヤーの弾に当たった場合はダメージ処理など
}