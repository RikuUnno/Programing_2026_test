#include "Bullet.h"
#include "Time.h"
#include "DxLib.h"
#include "ObjectManager.h"

Bullet::Bullet(const std::weak_ptr<SceneBase> scene, VECTOR position, float bulletRadiusSize)
	: CircleBase(scene), m_baseRadius(bulletRadiusSize)
{
	m_transform->SetPosition(position);
}

Bullet::~Bullet()
{
}

void Bullet::InitObject()
{
	CircleBaseInitUsingTransform(m_baseRadius);

	// コライダー設定
	m_collider.SetOwner(shared_from_this());
	m_collider.SetRadius(m_baseRadius);
	
	// デフォルトは0 (無効) にしておき、発射時にPlayer/Enemyが設定する
	m_collider.SetLayer(Layer::Default);
	m_collider.SetMask(0);

	m_collider.SetOnCollisionCallback([this](Collider* other) {
		this->OnHit(other);
	});
}

void Bullet::Start()
{
	m_collider.SetActive(true);
}

void Bullet::Update()
{

	// 移動処理
	float dt = static_cast<float>(Time::GetInstance().DeltaTime());
	VECTOR pos = m_transform->GetPosition();
	pos.x += m_direction.x * m_moveSpeed * dt;
	pos.y += m_direction.y * m_moveSpeed * dt;
	pos.z += m_direction.z * m_moveSpeed * dt;
	m_transform->SetPosition(pos);

	// Transform行列更新
	m_transform->UpdateMatrix();
	m_transform->LocalToWorldMatrix();

	// 形状更新
	CircleBaseInitUsingTransform(m_baseRadius);

	// 寿命処理
	m_age += dt;
	if (m_lifetime > 0.0f && m_age >= m_lifetime) {
		ObjectManager::GetInstance().Release(m_selfHandle);
		return;
	}

	
}

void Bullet::Draw()
{
	// 描画
	DrawStoredCircle(m_bulletColor, true);
}

void Bullet::End()
{
	// コライダー無効化
	m_collider.SetActive(false);
}

void Bullet::SetDirection(const VECTOR& dir)
{
	float len = VSize(dir);
	if (len > 0.0f) {
		m_direction = VScale(dir, 1.0f / len);
	}
}

VECTOR Bullet::GetVelocity() const
{
	return VScale(m_direction, m_moveSpeed);
}

void Bullet::OnHit(Collider* other)
{
	// 何かに当たったら消滅
	// (貫通弾の場合はここで消さない判定を入れる)
	if (m_selfHandle.IsValid()) {
		ObjectManager::GetInstance().Release(m_selfHandle);
	}
	// コライダー無効化
	m_collider.SetActive(false);
}
