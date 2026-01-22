#include "ObjectGroup.h"
#include "ObjectManager.h"
#include "DxLib.h"

// デバッグ出力をデバッグビルドのみに限定するマクロ
#if defined(_DEBUG) || defined(DEBUG)
	#define OG_DEBUG_PRINTF(...) printfDx(__VA_ARGS__)
#else
	#define OG_DEBUG_PRINTF(...) ((void)0)
#endif

// ハンドルを追加（無効ハンドルは無視）
// 追加時に対応する GameObject の Start() を一度だけ呼ぶように変更
void ObjectGroup::Add(ObjectHandle h)
{
	if (!h.IsValid()) return;

	{
		// コンテナ操作は短時間だけロック
		std::lock_guard<std::mutex> lk(m_mutex);
		m_handles.push_back(h);
	}

	// Start の呼び出しはロック外で行う（再入やデッドロックを回避）
	auto obj = ObjectManager::GetInstance().GetRaw(h);
	if (obj) {
		obj->Start();
	}
}

// ハンドルを削除（存在すれば）
void ObjectGroup::Remove(ObjectHandle h)
{
	std::lock_guard<std::mutex> lk(m_mutex);
	auto it = std::remove(m_handles.begin(), m_handles.end(), h);
	if (it != m_handles.end()) m_handles.erase(it, m_handles.end());
}

// 全クリア
void ObjectGroup::Clear()
{
	// スナップショットを作ってロック解放 → Release をロック外で行う
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
		m_handles.clear();
	}

	// スナップショットを使って Release を呼ぶ
	auto& mgr = ObjectManager::GetInstance();
	for (auto h : snapshot) {
		if (h.IsValid()) {	// 無効ハンドルは無視
			mgr.Release(h); // ロック外で呼ぶ
		}
	}
}

// 全オブジェクトを Update（無効ハンドルはリストから削除）
// m_handles の直接イテレーションは避け、短時間でスナップショットを取る
void ObjectGroup::UpdateAll()
{
	auto& mgr = ObjectManager::GetInstance(); // オブジェクトマネージャ取得

	// スナップショットを取得（短時間だけロック）
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
	}

	// スナップショット上で処理。無効なハンドルは削除フラグにする
	bool modified = false;
	std::vector<ObjectHandle> newList;
	newList.reserve(snapshot.size());

	// 各オブジェクトに対して Update を呼ぶ
	for (auto handle : snapshot)
	{
		auto obj = mgr.GetRaw(handle);
		if (obj)
		{
			// デバッグ出力
			OG_DEBUG_PRINTF("[ObjectGroup] UpdateAll handle idx=%u gen=%u ptr=%p use_count=%d\n",
					handle.index, handle.generation, obj.get(), static_cast<int>(obj.use_count()));

			if (obj.get() == nullptr || obj.use_count() <= 0)
			{
				// 無効扱い -> omit from new list
				modified = true;
				continue;
			}

			// Update を呼ぶ（ロック外）
			obj->Update();
			newList.push_back(handle);
		}
		else
		{
			// 解放済み -> omit
			modified = true;
		}
	}

	// コンテナが変更されたら置き換える（短時間ロック）
	if (modified)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		m_handles = std::move(newList);
	}
}

// 描画用（スナップショット方式）
void ObjectGroup::DrawAll()
{
	auto& mgr = ObjectManager::GetInstance(); // オブジェクトマネージャ取得

	// スナップショットを取得（短時間だけロック）
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
	}

	bool modified = false;
	std::vector<ObjectHandle> newList;
	newList.reserve(snapshot.size());

	// 各オブジェクトに対して Draw を呼ぶ
	for (auto handle : snapshot)
	{
		auto obj = mgr.GetRaw(handle);
		if (obj)
		{
			// デバッグ出力
			OG_DEBUG_PRINTF("[ObjectGroup] DrawAll handle idx=%u gen=%u ptr=%p use_count=%d\n",
					handle.index, handle.generation, obj.get(), static_cast<int>(obj.use_count()));

			if (obj.get() == nullptr || obj.use_count() <= 0)
			{
				modified = true;
				continue;
			}

			obj->Draw();
			newList.push_back(handle);
		}
		else
		{
			modified = true;
		}
	}

	if (modified) // コンテナが変更されたら置き換える
	{
		// 短時間ロック
		std::lock_guard<std::mutex> lk(m_mutex);
		m_handles = std::move(newList);
	}
}

// 全オブジェクトを End
void ObjectGroup::EndAll()
{
	// スナップショット取得（短時間ロック）
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
		// クリアも同時に行う
		m_handles.clear();
	}

	auto& mgr = ObjectManager::GetInstance();

	// 1) まず End を呼ぶ（ロック外）
	std::vector<std::shared_ptr<GameObject>> strongRefs;
	strongRefs.reserve(snapshot.size());
	for (auto h : snapshot) {
		if (!h.IsValid()) continue;
		if (auto obj = mgr.GetRaw(h)) {
			strongRefs.push_back(obj);
		}
	}

	// End 呼び出し
	for (auto& obj : strongRefs) {
		try {
			obj->End();
		}
		catch (const std::exception& e) { // 念のため例外キャッチ
			OutputDebugStringA(e.what());
		}
		catch (...) { // 不明な例外
			OutputDebugStringA("Unknown exception in End()\n");
		}
	}

	// End 呼び終わったらハンドルをマネージャに Release してプールへ戻す（ロック外）
	for (auto h : snapshot) {
		if (h.IsValid()) {
			mgr.Release(h);
		}
	}
}

// 任意の操作を各オブジェクトに対して行うヘルパー
void ObjectGroup::ForEach(const std::function<void(std::shared_ptr<GameObject>)>& func)
{
	auto& mgr = ObjectManager::GetInstance(); // オブジェクトマネージャ取得

	// スナップショット取得（短時間だけロック）
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
	}

	bool modified = false;
	std::vector<ObjectHandle> newList;
	newList.reserve(snapshot.size());

	// 各オブジェクトに対して func を呼ぶ
	for (auto handle : snapshot)
	{
		auto obj = mgr.GetRaw(handle);
		if (obj)
		{
			// デバッグ出力 
			OG_DEBUG_PRINTF("[ObjectGroup] ForEach handle idx=%u gen=%u ptr=%p use_count=%d\n",
					handle.index, handle.generation, obj.get(), static_cast<int>(obj.use_count()));

			if (obj.get() == nullptr || obj.use_count() <= 0)
			{
				modified = true;
				continue;
			}

			func(obj);
			newList.push_back(handle);
		}
		else
		{
			modified = true;
		}
	}

	// コンテナが変更されたら置き換える（短時間ロック）
	if (modified)
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		m_handles = std::move(newList);
	}
}

// 子オブジェクトを削除
void ObjectGroup::RemoveAllChild()
{
	auto& mgr = ObjectManager::GetInstance(); // オブジェクトマネージャ取得
	// スナップショット取得（短時間ロック）
	std::vector<ObjectHandle> snapshot;
	{
		std::lock_guard<std::mutex> lk(m_mutex);
		snapshot = m_handles;
	}

	// 各オブジェクトに対して子削除を呼ぶ
	for (auto handle : snapshot)
	{
		auto obj = mgr.GetRaw(handle); // オブジェクト取得
		if (obj)
		{
			// デバッグ出力
			OG_DEBUG_PRINTF("[ObjectGroup] RemoveAllChild handle idx=%u gen=%u ptr=%p use_count=%d\n",
					handle.index, handle.generation, obj.get(), static_cast<int>(obj.use_count()));

			// 無効ならスキップ
			if (obj.get() == nullptr || obj.use_count() <= 0)
			{
				continue;
			}
			obj->RemoveAllChild(); // 子オブジェクト全削除
		}
	}
}