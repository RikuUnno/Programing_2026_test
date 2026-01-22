#include "ChildTriangles.h"
#include "DxLib.h"
#include "Time.h"
#include "Bullet.h"
#include "ObjectManager.h"
#include "ColliderInfo.h" // 追加
#include <cmath>

// コンストラクタ
ChildTriangles::ChildTriangles(const std::weak_ptr<SceneBase> scene, VECTOR offset, float triangleRadiusSize)
	: TriangleBase(scene), m_offset(offset)
	, m_offsetSize(std::sqrt(offset.x* offset.x + offset.y * offset.y + offset.z * offset.z))
	, m_objectRadius(triangleRadiusSize)
{
	// 親の位置を基準に初期位置を設定（ローカル）
	m_transform->SetPosition({0.0f,0.0f,0.0f });
	m_transform->SetRotation({0.0f,0.0f,0.0f });
	m_transform->SetScale({1.0f,1.0f,1.0f });
	m_angle =0.0f;
	m_rotateSpeed =90.0f; //90度/秒

	// 初期三角形サイズ（スケール考慮は Update 側で再計算する）
	TriangleBaseInitUsingTransform(m_objectRadius,90.0f);
}

// デストラクタ
ChildTriangles::~ChildTriangles()
{
	// 弾を確実に解放（念のため）
	m_bulletTrigger.EndAll();
	m_bulletTrigger.Clear();
}

void ChildTriangles::InitObject()
{
	// プール再利用時の初期化
	m_fireTimer =0.0f;

	// コライダー設定
	m_collider.SetOwner(shared_from_this());
	m_collider.SetupFromBase(m_objectRadius, 180.0f);

	// レイヤー設定 (Enemy)
	m_collider.SetLayer(Layer::Enemy);
	
	// マスク設定: PlayerBullet(3) と Player(1) に当たる
	m_collider.SetMask((1 << Layer::PlayerBullet) | (1 << Layer::Player));

	// コールバック
	m_collider.SetOnCollisionEnterCallback([this](Collider* other) {
		this->OnHit(other);
	});
}

// 最初に一回だけ呼ばれる
void ChildTriangles::Start()
{
	m_isActive = true; // アクティブ化
	m_collider.SetActive(true); // コライダー有効化

	m_life = 100.0f; // 体力リセット

	// BulletTrigger の発射基準をこの子の Transform にする（所有はしない）
	m_bulletTrigger.SetOwnerObject(shared_from_this());

	// 発射タイマー初期化
	m_fireTimer =0.0f;
}

// 毎フレーム呼ばれる
void ChildTriangles::Update()
{
	if (!m_isActive) m_collider.SetActive(false); // コライダー無効化

	if(m_life <=0.0f) {
		m_isActive =false; // 非アクティブ化
		m_collider.SetActive(false); // コライダー無効化
		return;
	}

	Move(); // 移動処理を呼ぶ

	// 自身の三角形を transform に合わせて更新
	TriangleBaseInitUsingTransform(m_objectRadius,90.0f);

	// 子が管理する弾を更新
	m_bulletTrigger.Update();

	// 発射タイマー処理（自動射撃）
	float dt = static_cast<float>(Time::GetInstance().DeltaTime());
	m_fireTimer += dt;
	if (m_fireTimer >= m_fireRate) {
		Attack();
		m_fireTimer =0.0f;
	}

	// デバッグ描画
	if (m_collider.IsActive()) {
		m_collider.Draw();
	}
}

// 毎フレーム呼ばれる（描画用）
void ChildTriangles::Draw()
{
	if (m_isActive == false) return; // 非アクティブ時は処理しない

	// 自身の三角形描画
	DrawStoredTriangleEdges(GetColor(255,0,0), GetColor(0,255,0), GetColor(0,0,255));
	// 子が管理する弾の描画も行う
	m_bulletTrigger.Draw();
}

// シーンから削除されるときに一回だけ呼ばれる
void ChildTriangles::End()
{
	// 弾の終了処理
	m_bulletTrigger.EndAll();
	m_bulletTrigger.Clear();

	// コライダー無効化
	m_collider.SetActive(false);
}

// 移動処理(Class内)
void ChildTriangles::Move()
{
	// dt を取得
	float deltaTime = static_cast<float>(Time::GetInstance().DeltaTime());

	//角度更新（度）
	m_angle += m_rotateSpeed * deltaTime;
	if (m_angle >=360.0f) m_angle -=360.0f;
	else if (m_angle <0.0f) m_angle +=360.0f;

	const float PI =3.14159265f;
	float rad = m_angle * (PI /180.0f);

	// 回転したローカルオフセットを計算
	float rx = std::cos(rad) * m_offset.x - std::sin(rad) * m_offset.y;
	float ry = std::sin(rad) * m_offset.x + std::cos(rad) * m_offset.y;

	// ローカル位置として設定（親が設定されていれば親基準になる）
	m_transform->SetPosition(VGet(rx, ry, m_offset.z));

	// ローカル行列更新
	m_transform->UpdateMatrix();

	// 親の有無に関わらずワールド行列を更新（親がいると親ワールドと合成される）
	m_transform->LocalToWorldMatrix();
}

// 攻撃処理
void ChildTriangles::Attack()
{
	// 発射方向はローカルで指定（下方向）
	VECTOR dirLocal = VGet(0.0f,1.0f,0.0f);

	// 発射位置は子のローカル原点から子の半径分外側へ（子自身のオフセットや親と被らないように）
	const float margin =2.0f;
	VECTOR offsetLocal = VGet(0.0f, m_objectRadius + m_bulletRadius + margin,0.0f);

	// Shoot は BulletTrigger::Shoot(localOffset, direction) を呼び、ハンドルを返す
	auto h = m_bulletTrigger.Shoot(offsetLocal, dirLocal);
	if (!h.IsValid()) return;

	//生成直後に弾のパラメータを細かく調整
	ObjectManager& mgr = ObjectManager::GetInstance();
	auto b = mgr.Get<Bullet>(h);
	if (b) {
		b->SetSpeed(m_bulletSpeed);
		b->SetLifetime(m_bulletLife);
		b->SetBaseRadius(m_bulletRadius);
		b->SetColor(m_bulletColor);

		// 弾の設定
		b->SetLayer(Layer::EnemyBullet);
		b->SetMask(1 << Layer::Player);
	}
}

// 衝突コールバック
void ChildTriangles::OnHit(Collider* other)
{
	// ログ出力など
	m_life -= 10.0f; // 10ダメージ
}