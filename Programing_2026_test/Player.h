#pragma once
#include "TriangleBase.h"
#include "BulletTrigger.h"
#include "ObjectGroup.h"
#include "KeyInput.h"
#include "TriangleCollider.h" // 追加

class Player 
	: public TriangleBase, public KeyInput
{
private:
	// 移動用変数
	float m_moveSpeed = 200.0f; // 移動速度 (px/sec)
	float m_life = 100.0f;      // 体力

	// Object用
	float m_objectRadiusSize = 0.0f; // オブジェクトの半径サイズ
	
	// Child用変数
	// アイテム等の子オブジェクトを持つ場合にここに変数を追加
	ObjectGroup m_ChildObjectGroup;		// 子オブジェクト管理用

	// 攻撃用変数
	BulletTrigger m_bulletTrigger;	// 発射管理器（ハンドル管理のみ）
	float m_fireRate = 0.2f;		// 秒間発射間隔(秒)
	float m_fireTimer = 0.0f;		// 発射タイマー
	float m_bulletRadius = 3.0f;	// 弾の半径 (px/scale=1)
	float m_bulletSpeed = 300.0f;	// 弾の速度 (px/sec)
	float m_bulletLife = 10.0f;		// 弾の寿命 (秒)
	unsigned int m_bulletColor = GetColor(0, 255, 125); // 弾の色: 緑

	// 当たり判定用
	TriangleCollider m_collider; // 自分自身のコライダー

public:
	// コンストラクタ・デストラクタ
	Player(const std::weak_ptr<SceneBase> scene, VECTOR position, float objectRadiusSize);
	virtual ~Player();

	// GameObject ライフサイクル
	void InitObject() override;	// 追加: 初期化フック（生成直後に呼びたい初期化処理を派生クラスで実装）
	void Start() override;		// 最初に一回だけ呼ばれる
	void Update() override;		// 毎フレーム呼ばれる
	void Draw() override;		// 毎フレーム呼ばれる（描画用）
	void End() override;        // シーンから削除されるときに一回だけ呼ばれる

private:
	// 移動処理(Class内)
	void Move();
	// 攻撃処理(Class内)
	void Attack();
	// 防御処理(Class内)
	void Defend();
	
	// 衝突コールバック
	void OnHit(Collider* other);
};