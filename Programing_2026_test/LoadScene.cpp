#include "LoadScene.h"
#include "MainGameScene.h"
#include "DxLib.h"

LoadScene::LoadScene()
{

}

LoadScene::~LoadScene()
{

}

void LoadScene::Start()
{

}

void LoadScene::End()
{
    SceneBase::End(); // 基底クラスの End を呼ぶ(m_objects.Clear()を呼ぶ)
}

std::shared_ptr<SceneBase> LoadScene::Update()
{
	m_loadTimer += Time::GetInstance().DeltaTime(); // ロード時間をカウントアップ

    if (m_loadTimer > m_loadTime)
    {// 一定フレーム経過したら画面遷移
        return std::make_shared<MainGameScene>();
    }
    return shared_from_this();
}

void LoadScene::Draw()
{
    DrawString(0, 0, "LOADING...", 0xFFFFFF);
}