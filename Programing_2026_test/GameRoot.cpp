#include "DxLib.h"
#include "GameRoot.h"
#include "SceneBase.h"
#include "ObjectManager.h"
#include "GameScene.h"

#include "LoadScene.h"

GameRoot::GameRoot()
{
    m_scene = std::make_shared<LoadScene>();
}

GameRoot::~GameRoot()
{
    m_scene.reset();
}

void GameRoot::Start()
{

}

void GameRoot::End()
{
    SceneBase::End(); // 子シーンのオブジェクトも Endされているはず
}

std::shared_ptr<SceneBase> GameRoot::Update()
{
    auto next = m_scene->Update();
    if (next != m_scene)
    {
         //旧シーンの終了
         m_scene->End();

         // Scene が GameSceneでなければそのまま返す
         GameScene* scene = dynamic_cast<GameScene*>(next.get());
         if (scene == nullptr) {
             return next;
         }

         // 差し替え
         m_scene = next;
		 // 新シーンの開始
         ObjectManager::GetInstance().RegisterCurrentScene(m_scene);
         m_scene->Start();
    }

    return shared_from_this();
}


void GameRoot::Draw()
{
    if (m_scene) m_scene->Draw();
}