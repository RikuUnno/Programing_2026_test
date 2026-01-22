#include "SettingScene.h"
#include "TitleScene.h"
#include "DxLib.h"

SettingScene::SettingScene()
{

}

SettingScene::~SettingScene()
{

}

void SettingScene::Start()
{

}

void SettingScene::End()
{
    SceneBase::End();
}

std::shared_ptr<SceneBase> SettingScene::Update()
{
    BeginKeyInput();

    if (IsKeyInputTrigger(KEY_INPUT_T))
    {
        return std::make_shared<TitleScene>();
    }

    EndKeyInput();

    return shared_from_this();
}

void SettingScene::Draw()
{
    DrawString(0, 0, "SETTING MENU", 0xFFFFFF);
    DrawString(0, 20, "T RETURN", 0xFFFFFF);

}
