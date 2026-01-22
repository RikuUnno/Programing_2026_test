#pragma once
#include "SceneBase.h"
#include <memory>

class GameRoot
	: public SceneBase
{
private:
	std::shared_ptr<SceneBase> m_scene;

public:
	GameRoot();
	virtual ~GameRoot();

public:
	void Start() override;
	void End() override;

	std::shared_ptr<SceneBase> Update() override;

public:
	void Draw() override;
};