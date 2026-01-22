#include "MainGameScene.h"
#include "TitleScene.h"
#include "DxLib.h"
#include "ObjectManager.h"
#include "Triangles.h"
#include "Player.h"
#include "ColliderManager.h" // 追加

#include <algorithm>

MainGameScene::MainGameScene()
{
}

MainGameScene::~MainGameScene()
{
    m_objects.Clear(); // 全オブジェクトのクリア
}

void MainGameScene::Start()
{
    // Triangles を生成してシーンの ObjectGroup に登録
    // Scene を shared_ptr 管理に変更したため、現在アクティブなシーンの weak_ptr を渡す
	auto currentSceneWeak = SceneBase::GetCurrentSceneWeak(); // 現在のシーンの弱参照を取得(objectの生成時用)
    auto playerHandle = ObjectManager::GetInstance().Create<Player>(currentSceneWeak, VECTOR{ 320.0f, 360.0f, 0.0f }, 20.0f); // 画面下部に配置
    auto enemyHandle1 = ObjectManager::GetInstance().Create<Triangles>(currentSceneWeak, VECTOR{ 320.0f, 100.0f, 0.0f }, 35.0f); // 画面中央に配置
   
    // シーンの m_objects は protected なので同クラス内から追加
	m_objects.Add(playerHandle); // プレイヤー
	m_objects.Add(enemyHandle1); // 敵1
}

void MainGameScene::End()
{
	SceneBase::End(); // 基底クラスの End を呼ぶ(m_objects.Clear()を呼ぶ)
}

std::shared_ptr<SceneBase> MainGameScene::Update()
{
    Draw();

    // ObjectGroup に登録されたオブジェクトを更新
    m_objects.UpdateAll();

    // コライダーの更新
    // 全オブジェクトの座標更新(UpdateAll)が終わった後で行う
    ColliderManager::GetInstance().Execute();

    BeginKeyInput();

    if (IsKeyInputReleased(KEY_INPUT_Z))
    {
        return std::make_shared<TitleScene>();
    }

    EndKeyInput();

    return shared_from_this();
}

void MainGameScene::Draw()
{
    DrawString(0, 0, "MainGame!", 0xFFFFFF);
    DrawString(0, 20, "Z TITLE", 0xFFFFFF);

    // ObjectGroup に登録されたオブジェクトを描画
    m_objects.DrawAll(); // 全オブジェクトの描画
    
    // デバッグ用: コライダーの描画 (必要なら有効化)
    ColliderManager::GetInstance().DrawDebug();
}