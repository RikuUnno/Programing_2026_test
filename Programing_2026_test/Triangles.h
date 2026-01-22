// 敵１(Triangles) 
#pragma once
#include "TriangleBase.h"
#include "ObjectGroup.h"
#include "KeyInput.h"
#include "BulletTrigger.h" 
#include "TriangleCollider.h"
#include <memory>


class Triangles
	: public TriangleBase, public KeyInput
{
private:
	// Object用
	float m_objectRadiusSize = 0.0f; // オブジェクトの半径サイズ
	float m_life = 100.0f; // 体力

	// Move用変数
	int m_moveDir = 1; // 左右の移動方向
	int m_moveSpeed = 100; // 移動速度

	// Child用変数
	ObjectGroup m_ChildObjectGroup;
	float m_childObjectRadius = 30.0f;	// 子オブジェクトの半径
	float m_childOffsetSize = 80.0f;	// 子オブジェクトのオフセットの大きさ

	// Bullet 発射管理
	BulletTrigger m_bulletTrigger;	// 発射管理器（ハンドル管理のみ）
	float m_fireRate = 0.8f;		// 秒間発射間隔(秒)
	float m_fireTimer = 0.0f;		// 発射タイマー
	float m_bulletRadius = 3.0f;	// 弾の半径 (px/scale=1)
	float m_bulletSpeed = 100.0f;	// 弾の速度 (px/sec)
	float m_bulletLife = 10.0f;		// 弾の寿命 (秒)
	unsigned int m_bulletColor = GetColor(255, 0, 0); // 弾の色

	// 当たり判定用
	TriangleCollider m_collider;

public:
	Triangles(const std::weak_ptr<SceneBase> scene, VECTOR position, float objectRadiusSize);
	virtual ~Triangles();

public:
	void InitObject() override;	// 初期化フック（生成直後に呼びたい初期化処理を派生クラスで実装）
	void Start() override;		// 最初に一回だけ呼ばれる
	void Update() override;		// 毎フレーム呼ばれる
	void Draw() override;		// 毎フレーム呼ばれる（描画用）
	void End() override;        // シーンから削除されるときに一回だけ呼ばれる

private:
	void Move(); // 移動処理(Class内)

	void Attack(); // 攻撃処理(Class内)

	void OnHit(Collider* other); // 衝突処理
};
