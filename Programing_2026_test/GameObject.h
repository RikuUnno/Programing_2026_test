#pragma once
#include "Transform.h"
#include <vector>
#include <memory>
#include <string>

// 前方宣言
class SceneBase;

class GameObject : public std::enable_shared_from_this<GameObject>
{
protected:
	std::vector<std::shared_ptr<GameObject>> m_child;	// 子供オブジェクトのリスト
	std::weak_ptr<SceneBase> m_scene;				// 所属しているシーンへの参照

	// ソート順など
	std::string m_name = "None"; // オブジェクト名
	int m_tag = 0;    // オブジェクトタグ
	int m_layer = 0;  // オブジェクトレイヤー
	int m_id = -1;   // オブジェクトID

	bool m_isActive = true; // アクティブフラグ

protected:
	std::shared_ptr<Transform> m_transform = std::make_shared<Transform>(); // 位置情報

public:
	GameObject();										// デフォルトコンストラクタ
	GameObject(const std::weak_ptr<SceneBase> scene);	// シーンを受け取るコンストラクタ
	virtual ~GameObject();								// デストラクタ

public:
	virtual void InitObject();	// 初期化フック（生成直後に呼びたい初期化処理）
	virtual void Start();		// 最初に一回だけ呼ばれる
	virtual void Update();		// 毎フレーム呼ばれる
	virtual void Draw();		// 毎フレーム呼ばれる（描画用）
	virtual void End();        // シーンから削除されるときに一回だけ呼ばれる

public: // ゲッター
	std::shared_ptr<Transform> GetTransform() { return m_transform; } // Transformの取得
	// m_scene のゲッター 
	std::shared_ptr<SceneBase> GetSceneShared() const { return m_scene.lock(); }

	// シーン参照の取得/設定
	std::weak_ptr<SceneBase> GetSceneWeak() const { return m_scene; }
	void SetScene(const std::weak_ptr<SceneBase>& s) { m_scene = s; } // シーンの設定

public:
	void AddChild(std::shared_ptr<GameObject> child); // 子の追加
	void RemoveChild(std::shared_ptr<GameObject> child); // 子の削除(単体)

	void RemoveAllChild(); // 子の削除(全消)

public:
	void IsActive(bool active) { m_isActive = active; } // アクティブ設定
	bool GetIsActive() const { return m_isActive; }        // アクティブ取得
};