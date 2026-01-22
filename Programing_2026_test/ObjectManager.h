#pragma once
#include "ObjectPool.h"
#include "ObjectHandle.h"
#include "GameObject.h"
#include "Assert.h"
#include "SceneBase.h"
#include <memory>
#include <type_traits>
#include <windows.h>


class ObjectManager
{
public:
	// シングルトンインスタンスの取得
	static ObjectManager& GetInstance();

	// Create (テンプレート、ヘッダ実装)
	template <typename T, typename... Args>
	ObjectHandle Create(Args&&... args)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject"); // T が GameObject 派生であることを確認
		return m_pool.Acquire<T>(std::forward<Args>(args)...); // オブジェクトプールからハンドルを取得
	}

	// Get typed : ハンドルから shared_ptr<T> を取得
	template <typename T>
	std::shared_ptr<T> Get(ObjectHandle handle)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject"); // T が GameObject 派生であることを確認
		return m_pool.Get<T>(handle); // ハンドルから shared_ptr<T> を取得
	}

	// Get raw : そのまま shared_ptr<GameObject> を取得
	std::shared_ptr<GameObject> GetRaw(ObjectHandle handle)
	{
		return m_pool.Get(handle);
	}

	// Release : ハンドルに対応するオブジェクトをプールに戻す
	bool Release(ObjectHandle handle)
	{
		// デバッグログ出力
		char buf[128];
		sprintf_s(buf, "[ObjectManager] Release called idx=%u gen=%u\n", handle.index, handle.generation);
		OutputDebugStringA(buf);
		return m_pool.Release(handle);
	}

	// Utility
	bool IsValid(ObjectHandle handle) { return m_pool.IsHandleValid(handle); }	// ハンドルの有効性チェック
	size_t PoolSize() const { return m_pool.PoolSize(); }						// プールサイズ取得
	size_t FreeCount() const { return m_pool.FreeCount(); }					// 空きスロット数取得

	// 全破棄（実体は cpp 側で実装）
	void ClearAll();

	// デバッグ表示: 現在のアクティブオブジェクト数を画面に描画する
	void DrawObjectCount(int x, int y) const;

	// 自動クリーンアップ（ラッパ）
	size_t CleanupIdle(double maxIdleSeconds)
	{
		return m_pool.CleanupIdle(maxIdleSeconds);
	}

	// デバッグ用状態出力
	void DumpState() const { m_pool.DumpState(); }

	// 更新: 全オブジェクトの scene weak_ptr を更新する（シーン切替時に呼ぶ）
	void UpdateAllObjectsScene(const std::weak_ptr<SceneBase>& scene);

	// 現在のシーンを登録してプール内オブジェクトの scene を更新するユーティリティ
	void RegisterCurrentScene(const std::shared_ptr<SceneBase>& scene);

private:
	// シングルトンなのでコンストラクタ類は非公開
	ObjectManager() = default;
	~ObjectManager() = default;
	ObjectManager(const ObjectManager&) = delete;
	ObjectManager& operator=(const ObjectManager&) = delete;

private:
	ObjectPool m_pool; // オブジェクトプール
};