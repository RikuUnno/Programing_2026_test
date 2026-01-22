#include "Player.h"
#include "Time.h"
#include "DxLib.h"
#include "ObjectManager.h"
#include "Bullet.h"
#include "ColliderInfo.h"
#include <algorithm>
#include <cmath>

// コンストラクタ
Player::Player(const std::weak_ptr<SceneBase> scene, VECTOR position, float objectRadiusSize)
	: TriangleBase(scene), KeyInput(),				// 親クラスコンストラクタ呼び出し
	m_objectRadiusSize(objectRadiusSize) 			// オブジェクトの半径サイズ設定
{
	m_transform->SetPosition(position); // 初期値を設定
	m_transform->SetRotation({0.0f,0.0f,0.0f });
	m_transform->SetScale({1.0f,1.0f,1.0f });

	// Child 用変数初期化
	// あればここに追加

	// 初期三角形サイズ（スケール考慮は Update 側で再計算する）
	TriangleBaseInitUsingTransform(m_objectRadiusSize,270.0f);
}

// デストラクタ
Player::~Player()
{
	m_ChildObjectGroup.Clear(); // Release 管理は End()で行うが念のため
	m_bulletTrigger.Clear();	// Release 管理は End()で行うが念のため
}

// 初期化フック（生成直後に呼びたい初期化処理を派生クラスで実装）
void Player::InitObject()
{
	// コンストラクタから移動した処理
	m_bulletTrigger.SetOwnerObject(shared_from_this());

	// 三角形の初期化（大きさ20）
	TriangleBaseInitUsingTransform(20.0f);

	// コライダー設定
	// 所有者を自分に設定
	m_collider.SetOwner(shared_from_this());
	// 形状設定 (三角形のサイズに合わせる)
	m_collider.SetupFromBase(m_objectRadiusSize, 0.0f);	

	// 3. レイヤー設定 (Player = 1)
	m_collider.SetLayer(Layer::Player);
	
	// 4. マスク設定 (Enemy(2) と EnemyBullet(4) と当たりたい)
	m_collider.SetMask((1 << Layer::Enemy) | (1 << Layer::EnemyBullet));

	// 5. コールバック登録 ()
	m_collider.SetOnCollisionEnterCallback([this](Collider* other){
		this->OnHit(other);
	});
}

// 最初に一回だけ呼ばれる
void Player::Start()
{
	m_isActive = true; // アクティブ化
	m_collider.SetActive(true); // コライダー有効化

	m_life = 100.0f; // 体力初期化

	// BulletTrigger の所有 Transform を保証（ここでも設定可能)
	m_bulletTrigger.SetOwnerObject(shared_from_this());

	
}

// 毎フレーム呼ばれる
void Player::Update()
{
	if (m_isActive) { // 非アクティブ時は処理しない

		if (m_life < 0.0f) {
			// 死亡処理
			IsActive(false); // 非アクティブ化
			m_collider.SetActive(false); // コライダー無効化
			return;
		};

		// 移動処理
		Move();

		// Transform の行列を更新しておく（移動が無いフレームでもワールド位置を反映する）
		m_transform->UpdateMatrix();
		m_transform->LocalToWorldMatrix();

		// 発射タイマー更新
		float dt = static_cast<float>(Time::GetInstance().DeltaTime());
		if (m_fireTimer > 0.0f) m_fireTimer -= dt;

		// 左クリックで射出（押しっぱなしで連射は発射間隔で制御）
		if ((GetMouseInput() & MOUSE_INPUT_LEFT) != 0 && m_fireTimer <= 0.0f)
		{
			Attack();
			m_fireTimer = m_fireRate;
		}

		// 三角形の表示更新（Transform とスケールを考慮）
		TriangleBaseInitUsingTransform(m_objectRadiusSize, 270.0f);

		// 子の更新
		m_ChildObjectGroup.UpdateAll();

	}

	// 弾の更新
	m_bulletTrigger.Update();
}

// 毎フレーム呼ばれる（描画用）
void Player::Draw()
{
	if (m_isActive == false) return; // 非アクティブ時は処理しない

	// 三角形の描画
	DrawStoredTriangleEdges(GetColor(255,255,255), GetColor(255,255,255), GetColor(255,255,255)); // 色: 白
	m_ChildObjectGroup.DrawAll();	// 子の描画
	m_bulletTrigger.Draw();		// 弾の描画
}

// シーンから削除されるときに一回だけ呼ばれる
void Player::End()
{
	m_ChildObjectGroup.RemoveAllChild(); // 子の親子関係解除
	m_ChildObjectGroup.EndAll(); // 子の終了処理
	m_ChildObjectGroup.Clear(); // 子の解放
	
	// コライダーの無効化 (Managerからも登録解除される)
	m_collider.SetActive(false);
}

// 移動処理(Class内)
void Player::Move()
{
	float vx =0.0f;
	float vy =0.0f;

	// 左右（矢印 / A D）
	if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_A)) vx -=1.0f;
	if (CheckHitKey(KEY_INPUT_RIGHT) || CheckHitKey(KEY_INPUT_D)) vx +=1.0f;
	// 上下（矢印 / W S）
	// DxLib の座標系では y が下方向に増えるため、上方向は -1、下方向は +1
	if (CheckHitKey(KEY_INPUT_UP) || CheckHitKey(KEY_INPUT_W)) vy -=1.0f;
	if (CheckHitKey(KEY_INPUT_DOWN) || CheckHitKey(KEY_INPUT_S)) vy +=1.0f;

	// ノーモーション
	if (vx ==0.0f && vy ==0.0f) return;

	// 対角移動時に速度が速くならないよう正規化
	float len = std::sqrt(vx * vx + vy * vy);
	if (len >0.0f) {
		vx /= len;
		vy /= len;
	}

	// フレーム時間を考慮して移動量を計算
	float dt = static_cast<float>(Time::GetInstance().DeltaTime());
	VECTOR pos = m_transform->GetPosition();
	pos.x += vx * m_moveSpeed * dt;
	pos.y += vy * m_moveSpeed * dt;

	//画面端で制限する（オブジェクト半径をマージンとして使用）
	int winW =0, winH =0;
	GetWindowSize(&winW, &winH);

	// 有効なウィンドウサイズでなければ既定の値を使う
	if (winW <=0) winW =1280;
	if (winH <=0) winH =720;

	// オブジェクトの見た目サイズ（スケールを考慮）
	float scaleX = m_transform->GetScale().x;
	float scaleY = m_transform->GetScale().y;
	float marginX = std::abs(m_objectRadiusSize * scaleX);
	float marginY = std::abs(m_objectRadiusSize * scaleY);

	// clamp
	float minX = marginX;
	float maxX = static_cast<float>(winW) - marginX;
	float minY = marginY;
	float maxY = static_cast<float>(winH) - marginY;

	// 安全: margin が大きすぎる場合は中央に固定
	if (minX > maxX) {
		minX = maxX = static_cast<float>(winW) *0.5f;
	}
	if (minY > maxY) {
		minY = maxY = static_cast<float>(winH) *0.5f;
	}

	// 制限：画面端
	pos.x = (std::min)((std::max)(pos.x, minX), maxX);
	pos.y = (std::min)((std::max)(pos.y, minY), maxY);

	//反映
	m_transform->SetPosition(pos);
	m_transform->UpdateMatrix();
	m_transform->LocalToWorldMatrix();
}

// 攻撃処理(Class内)
void Player::Attack()
{
	// 発射方向はローカル Y+（プレイヤーの向きに合わせる）
	VECTOR dirLocal = VGet(0.0f,-1.0f,0.0f);
	// オフセットはローカル Y方向に baseRadius+bulletRadius+margin
	float margin =0.0f;
	VECTOR offsetLocal = VGet(0.0f, -(m_objectRadiusSize + m_bulletRadius + margin),0.0f);

	// 発射
	auto h = m_bulletTrigger.Shoot(offsetLocal, dirLocal);
	if (!h.IsValid()) return;

	//生成後に弾パラメータを設定
	if (auto b = ObjectManager::GetInstance().Get<Bullet>(h)) {
		b->SetSpeed(m_bulletSpeed);
		b->SetLifetime(m_bulletLife);
		b->SetBaseRadius(m_bulletRadius);
		b->SetColor(m_bulletColor);
		
		// ★Playerの弾として設定
		b->SetLayer(Layer::PlayerBullet); 
		// Mask = Enemy(2) に当たる
		// 自分の Layer(1) とは判定しないため、自機には当たらない
		b->SetMask(1 << Layer::Enemy); 
	}
}

// コールバックの実装
void Player::OnHit(Collider* other)
{
	m_life -= 10.0f; // 10ダメージ
}

// 防御処理(Class内)
void Player::Defend()
{
	//ここに防御処理を実装
}
