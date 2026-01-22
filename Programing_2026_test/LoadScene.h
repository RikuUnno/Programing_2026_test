#pragma once
#include "GameScene.h"

class LoadScene
	: public GameScene
{
private:
	// ロード時間関連(仮)
	float m_loadTime = 2;  // ロードにかかる時間(秒)
	float m_loadTimer = 0; // ロード時間計測用タイマー

public:
	LoadScene();
	virtual ~LoadScene();

public:
	void Start() override;
	void End() override;

	std::shared_ptr<SceneBase> Update() override;

public:
	void Draw() override;
};