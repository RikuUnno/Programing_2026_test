#pragma once
#include "SceneBase.h"
#include "CircleBase.h"
#include "ObjectHandle.h"
#include "CircleCollider.h" // 追加
#include <memory>

class Bullet
	: public CircleBase
{
private:
	
	float m_baseRadius = 8.0f;						// baseRadius（scale=1 のときの半径）
	float m_moveSpeed = 500.0f;						// 移動速度（ピクセル/秒）
	VECTOR m_direction = { 1.0f, 0.0f, 0.0f };		// 正規化された移動方向ベクトル
	float m_lifetime = 5.0f;						// 生存時間（秒）。0 または負なら無制限
	float m_age = 0.0f;								// 経過時間（秒)
	ObjectHandle m_selfHandle;						// 自分のハンドル（生成元がセットする）
	float m_outMargin = 16.0f;						// 画面外判定用マージン（半径などを考慮）
	unsigned int m_bulletColor = GetColor(255, 255, 255);	// 弾の色(デフォルト白)

	// 当たり判定
	CircleCollider m_collider;

public:
	// コンストラクタ: scene と初期ワールド位置、base radius（scale=1 の半径）
	Bullet(const std::weak_ptr<SceneBase> scene, VECTOR position, float bulletRadiusSize);
	virtual ~Bullet();

	// GameObject ライフサイクル
	void InitObject() override;
	void Start() override;
	void Update() override;
	void Draw() override;
	void End() override;

	// 設定 API（生成直後に呼ぶ）
	void SetDirection(const VECTOR& dir); // 正規化して保存
	void SetSpeed(float speed) { m_moveSpeed = speed; }
	void SetLifetime(float seconds) { m_lifetime = seconds; m_age = 0.0f; }
	void SetSelfHandle(ObjectHandle h) { m_selfHandle = h; }
	void SetOutMargin(float m) { m_outMargin = m; }
	void SetBaseRadius(float r) { m_baseRadius = r; m_collider.SetRadius(r); } // コライダーも更新
	void SetColor(unsigned int color) { m_bulletColor = color; }

	// 外部からレイヤー/マスクを設定できるようにする
	void SetLayer(uint32_t layer) { m_collider.SetLayer(layer); }
	void SetMask(uint32_t mask) { m_collider.SetMask(mask); }

	void SetColActive(bool active) { m_collider.SetActive(active); }

	// 便利: ワールド座標での速度ベクトルを取得
	VECTOR GetVelocity() const;

private:
	void OnHit(Collider* other);
};