#include "DxLib.h"
#include "Time.h"
#include "SceneBase.h"
#include "TitleScene.h"
#include "ObjectManager.h"
#include <chrono>
#include <memory>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ChangeWindowMode(TRUE); // ウィンドウモード

	SetGraphMode(1280,720,32); //画面解像度

	SetWaitVSyncFlag(FALSE); // VSync オフ

	if (DxLib_Init() == -1) // DxLib 初期化
	{
		return -1;
	}

	// シーンを shared_ptrで管理（shared_from_this を使えるようにする）
	std::shared_ptr<SceneBase> pRootScene = std::make_shared<TitleScene>();
	// ObjectManager 経由で初期シーンを登録する
	ObjectManager::GetInstance().RegisterCurrentScene(pRootScene);
	pRootScene->Start();

	// 裏画面を描画対象に
	SetDrawScreen(DX_SCREEN_BACK);

	double cleanupInterval =10.0; // 解放チェック間隔

	// メインループ
	while (CheckHitKey(KEY_INPUT_ESCAPE) ==0 && ProcessMessage() ==0)
	{
		clsDx();
		ClearDrawScreen();

		Time::GetInstance().Update();
		double dt = Time::GetInstance().DeltaTime();

		// シーン更新：Update は shared_ptr を返す
		auto nextPtr = pRootScene->Update();
		if (nextPtr.get() != pRootScene.get())
		{
			pRootScene->End();
			pRootScene = nextPtr; // shared_ptr に置き換え
			// ObjectManager に新シーンを登録
			ObjectManager::GetInstance().RegisterCurrentScene(pRootScene);
			pRootScene->Start();
		}

		// オブジェクトプールのクリーンアップ
		size_t removed = ObjectManager::GetInstance().CleanupIdle(cleanupInterval);

		// 描画
		pRootScene->Draw();

		// デバッグ: オブジェクト数表示
		ObjectManager::GetInstance().DrawObjectCount(3,700);

		ScreenFlip();
	}

	// 終了処理
	if (pRootScene) { pRootScene->End(); pRootScene.reset(); }

	// オブジェクトマネージャの解放
	ObjectManager::GetInstance().ClearAll();

	// DxLib 終了
	DxLib_End();

	return 0;
}
