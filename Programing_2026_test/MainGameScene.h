#pragma once
#include "GameScene.h"
#include <memory>

class MainGameScene
	: public GameScene
{
public:
	MainGameScene();
	virtual ~MainGameScene();

public:
	void Start() override;
	void End() override;

	std::shared_ptr<SceneBase> Update() override;

public:
	void Draw() override;

};
