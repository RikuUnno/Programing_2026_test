#pragma once
#include "SceneBase.h"
#include <memory>

class SettingScene
	: public SceneBase
{
public:
	SettingScene();
	virtual ~SettingScene();

public:
	void Start() override;
	void End() override;

	std::shared_ptr<SceneBase> Update() override;

public:
	void Draw() override;
};