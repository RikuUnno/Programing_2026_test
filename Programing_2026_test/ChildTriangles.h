#pragma once
#include "TriangleBase.h"
#include "BulletTrigger.h"
#include "TriangleCollider.h"

class ChildTriangles
	: public TriangleBase
{
private:
	// Object用
	float m_life = 100.0f; // 体力

	// Move用変数
	VECTOR m_offset = { 0.0f, 0.0f, 0.0f }; // 親からのオフセット位置
	float m_rotateSpeed = 20.0f;					// 回転速度
	float m_angle = 0.0f;						// 現在の角度

	// オフセット用変数
	float m_offsetSize = 0.0f;               // オフセットの大きさ
	float m_objectRadius = 0.0f;             // オブジェクトの半径

	// Bullet 発射管理
	BulletTrigger m_bulletTrigger;	// 発射管理器（ハンドル管理のみ）
	float m_fireRate = 0.8f;		// 秒間発射間隔(秒)
	float m_fireTimer = 0.0f;		// 発射タイマー
	float m_bulletRadius = 3.0f;	// 弾の半径 (px/scale=1)
	float m_bulletSpeed = 100.0f;	// 弾の速度 (px/sec)
	float m_bulletLife = 10.0f;		// 弾の寿命 (秒)
	unsigned int m_bulletColor = GetColor(0, 173, 173); // 弾の色(シアン系)

	// 当たり判定用
	TriangleCollider m_collider;

public:
	ChildTriangles(const std::weak_ptr<SceneBase> scene, VECTOR offset, float triangleRadiusSize);
	virtual ~ChildTriangles();

public:
	void InitObject() override;	// 追加: 初期化フック（生成直後に呼びたい初期化処理を派生クラスで実装）
	void Start() override;		// 最初に一回だけ呼ばれる
	void Update() override;		// 毎フレーム呼ばれる
	void Draw() override;		// 毎フレーム呼ばれる（描画用）
	void End() override;        // シーンから削除されるときに一回だけ呼ばれる

private:
	void Move(); // 移動処理(Class内)

	void Attack(); // 攻撃処理(Class内)

	// 衝突コールバック
	void OnHit(Collider* other);

public:
	float GetOffsetSize() const { return m_offsetSize; } // オフセットの大きさを取得
	float GetObjectRadius() const { return m_objectRadius; } // オブジェクトの半径を取得
};