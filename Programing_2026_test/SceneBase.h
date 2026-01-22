#pragma once
#include "KeyInput.h"
#include "ObjectGroup.h"
#include <memory>
#include <vector>

class SceneBase
	: public KeyInput, public std::enable_shared_from_this<SceneBase> // シーン自身の shared_ptr を取得可能にする	
{
public:
	SceneBase() {}
	virtual ~SceneBase() {}

public:
	virtual void Start() =0;

	// シーン内のオブジェクトを全クリア
	virtual void End() { m_objects.EndAll(); m_objects.Clear(); }

	// Update は次のアクティブシーンを shared_ptrで返す（このシーン継続は shared_from_this() を返す）
	virtual std::shared_ptr<SceneBase> Update() =0;

public:
	virtual void Draw() =0;

	// 現在アクティブなシーンをグローバルに参照するためのヘルパー
	static void SetCurrentScene(const std::shared_ptr<SceneBase>& scene);
	// 現在アクティブなシーンの弱参照を取得
	static std::weak_ptr<SceneBase> GetCurrentSceneWeak();

protected:
	// シーン内のオブジェクト管理用
	ObjectGroup m_objects; // オブジェクトグループ

	// 現在アクティブなシーンの弱参照用
	static std::weak_ptr<SceneBase> s_currentScene;
};
