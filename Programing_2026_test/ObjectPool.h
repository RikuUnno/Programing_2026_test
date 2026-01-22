#pragma once
#include "DxLib.h"
#include "ObjectHandle.h"
#include "GameObject.h"
#include "Factory.h"
#include "Assert.h"
#include <vector>
#include <memory>
#include <mutex>
#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <chrono>
#include <typeinfo>
#include <windows.h>
#include <sstream>
#include <cstdarg>
#include <atomic>

// デバッグログ出力ヘルパー
static inline void DebugLogFmt(const char* fmt, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
	va_end(ap);
	OutputDebugStringA(buf);
}

class ObjectPool
{
private:
	// アクティブ操作ガード: コンストラクタでカウンタをインクリメントし、デストラクタでデクリメントする
	struct ActivityGuard {
		std::atomic<int>& counter;
		ActivityGuard(std::atomic<int>& c) : counter(c) { counter.fetch_add(1, std::memory_order_relaxed); }
		~ActivityGuard() { counter.fetch_sub(1, std::memory_order_relaxed); }
	};

	// スロット情報
	struct Slot
	{
		std::shared_ptr<GameObject> obj; // 実体（nullptrなら未保持）
		uint32_t generation =0; // 世代番号
		bool inUse = false; // 使用中フラグ
		std::chrono::steady_clock::time_point lastUsed = std::chrono::steady_clock::now(); // 最終使用時刻
	};

public:
	ObjectPool() = default;
	~ObjectPool() = default;

	// Acquire: プールから取得（再利用 or 新規生成）
	template <typename T, typename... Args>
	ObjectHandle Acquire(Args&&... args)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");

		// シャットダウン中は取得を拒否
		if (m_shuttingDown.load(std::memory_order_acquire)) {
			DebugLogFmt("[ObjectPool] Acquire refused: shutting down\n");
			return ObjectHandle(); // invalid
		}
		ActivityGuard ag(m_activeOps);

		using Clock = std::chrono::steady_clock;
		auto now = Clock::now();

		// ロックを取得
		std::unique_lock<std::mutex> lk(m_mutex);

		uint32_t idx =0;

		//まずは空きリストからインデックスを取得
		if (!m_freeIndices.empty())
		{
			auto it = std::find_if(m_freeIndices.begin(), m_freeIndices.end(), [&](uint32_t i) {
				if (i >= m_slots.size()) return false;
				auto& s = m_slots[i];
				if (!s.obj) return false;
				return static_cast<bool>(std::dynamic_pointer_cast<T>(s.obj));
				});

			if (it != m_freeIndices.end()) {
				idx = *it;
				m_freeIndices.erase(it);
				DebugLogFmt("[ObjectPool] Reserve idx=%u (reusing same-type free). freeCount=%zu\n", idx, m_freeIndices.size());
			}
			else {
				// fallback: take last free index
				idx = m_freeIndices.back();
				m_freeIndices.pop_back();
				DebugLogFmt("[ObjectPool] Reserve idx=%u (popped). freeCount=%zu\n", idx, m_freeIndices.size());
			}
		}
		else
		{
			// 空きリストが無い場合はスキャンして再利用可能なオブジェクトを探す
			bool reused = false;
			for (uint32_t i =0; i < m_slots.size(); ++i) {
				Slot& s = m_slots[i];
				if (!s.inUse && s.obj && s.obj.use_count() ==1) {
					auto existing = std::dynamic_pointer_cast<T>(s.obj);
					if (existing) {
						idx = i;
						reused = true;
						DebugLogFmt("[ObjectPool] Reserve idx=%u (scan reuse). slots=%zu\n", idx, m_slots.size());
						// 再利用予約
						s.inUse = true;
						s.lastUsed = now;
						// 再利用: 再初期化が必要なので InitObject を呼ぶ
						try { s.obj->InitObject(); }
						catch (...) { DebugLogFmt("[ObjectPool] Exception during InitObject on scanned reuse idx=%u\n", idx); }
						return ObjectHandle(idx, s.generation);
					}
				}
			}

			if (!reused) {
				idx = static_cast<uint32_t>(m_slots.size());
				m_slots.emplace_back();
				DebugLogFmt("[ObjectPool] Reserve idx=%u (new slot). slots=%zu\n", idx, m_slots.size());
			}
		}

		// 他のスレッドが同じスロットを再利用できないように予約済みフラグを立てる
		m_slots[idx].inUse = true;
		m_slots[idx].lastUsed = now;
		uint32_t curGen = m_slots[idx].generation;

		//既存オブジェクトが同じ型なら再利用
		if (m_slots[idx].obj)
		{
			auto existing = std::dynamic_pointer_cast<T>(m_slots[idx].obj);
			if (existing)
			{
				DebugLogFmt("[ObjectPool] Reuse slot idx=%u gen=%u type=%s\n", idx, curGen, typeid(*m_slots[idx].obj).name());
				// 再利用: 再初期化が必要なので InitObject を呼ぶ
				try {
					m_slots[idx].obj->InitObject();
				}
				catch (...) {
					DebugLogFmt("[ObjectPool] Exception during InitObject on reuse idx=%u\n", idx);
				}
				m_slots[idx].lastUsed = Clock::now();
				return ObjectHandle(idx, curGen);
			}
			else {
				DebugLogFmt("[ObjectPool] Slot idx=%u has object of type=%s but requested type=%s\n", idx, typeid(*m_slots[idx].obj).name(), typeid(T).name());
			}
		}

		// ロックを外して新しいオブジェクトを生成する
		lk.unlock();

		// 新規生成
		auto createdObj = Factory::GetInstance().CreateObject<T>(std::forward<Args>(args)...);

		// 再度ロックを取得
		std::lock_guard<std::mutex> guard(m_mutex);

		// スロットが増えている可能性があるので再確認
		if (idx >= m_slots.size())
		{
			// 新しいスロットを追加
			idx = static_cast<uint32_t>(m_slots.size());
			m_slots.emplace_back();
			m_slots[idx].inUse = true;
			m_slots[idx].lastUsed = now;
		}

		// 新規生成なので生産累計を増やす
		++m_totalCreated;

		//もし既にスロットにオブジェクトが残っていて、そのオブジェクトがプールのみの所有（use_count==1）なら
		// 新しいオブジェクトで上書きすることで実際に解放が行われるため、削除カウントを増やす
		if (m_slots[idx].obj) {
			if (m_slots[idx].obj.use_count() ==1) {
				++m_totalDeleted;
			}
		}

		m_slots[idx].obj = std::static_pointer_cast<GameObject>(createdObj);
		m_slots[idx].lastUsed = Clock::now();
		// 初期化フックを呼ぶ（プールへ格納した直後に呼ぶ）
		if (m_slots[idx].obj) {
			m_slots[idx].obj->InitObject();
		}
		DebugLogFmt("[ObjectPool] Create stored idx=%u gen=%u slots=%zu free=%zu totalCreated=%zu totalDeleted=%zu\n", idx, m_slots[idx].generation, m_slots.size(), m_freeIndices.size(), m_totalCreated, m_totalDeleted);

		return ObjectHandle(idx, m_slots[idx].generation);
	}

	// Get: ハンドルから shared_ptr<T> を取得（無効なら nullptr）
	template <typename T>
	std::shared_ptr<T> Get(ObjectHandle handle)
	{
		// シャットダウン中は参照を返さない
		if (m_shuttingDown.load(std::memory_order_acquire)) {
			return nullptr;
		}
		ActivityGuard ag(m_activeOps); // アクティブ操作ガード

		// 型チェック
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");

		std::lock_guard<std::mutex> lk(m_mutex);

		if (!IsHandleValid_NoLock(handle)) return nullptr; // 無効ハンドル

		Slot& slot = m_slots[handle.index];
		// アクセスを「使用」とみなして lastUsed を更新（短期参照のアクセスを記録）
		slot.lastUsed = std::chrono::steady_clock::now();
		return std::dynamic_pointer_cast<T>(slot.obj);
	}

	// Get : ハンドルから shared_ptr<GameObject> を取得（無効なら nullptr）
	std::shared_ptr<GameObject> Get(ObjectHandle handle);

	// Release: ハンドルに対応するオブジェクトをプールに戻す（所有は slot.obj に残す）
	bool Release(ObjectHandle handle);

	// ClearAll: 全スロットをクリア（終了時用）
	void ClearAll();

	// 自動クリーンアップ: 指定秒以上アイドルのスロットを解放する
	size_t CleanupIdle(double maxIdleSeconds);

	// 有効性チェック（外部呼び出し用）
	bool IsHandleValid(ObjectHandle handle);

	// プール全体のスロット数を返す
	size_t PoolSize() const;

	// アクティブオブジェクト数を返す
	size_t ActiveCount() const;

	// 空きスロット数を返す
	size_t FreeCount() const;

	// 現在のスロット状態をログ出力（デバッグ用）
	void DumpState() const;

	// 総生産カウンタ（スレッドセーフな読み取りを提供）
	size_t TotalCreated() const;

	// 総削除カウンタ（スレッドセーフな読み取りを提供）
	size_t TotalDeleted() const;

	// Update scene weak_ptr for all live objects (called when active scene changes)
	void UpdateAllObjectsScene(const std::weak_ptr<SceneBase>& scene);

private:
	// ロック無し版の有効性チェック
	bool IsHandleValid_NoLock(ObjectHandle handle) const;

private:
	mutable std::mutex m_mutex; // スレッドセーフ用ミューテックス
	std::vector<Slot> m_slots; // スロット配列
	std::vector<uint32_t> m_freeIndices; // 空きスロットインデックスリスト

	// シャットダウン／アクティブ操作管理
	std::atomic<bool> m_shuttingDown{ false };
	std::atomic<int> m_activeOps{0 };

	//これまでに生成されたオブジェクト総数（累計）
	size_t m_totalCreated =0;
	//これまでに解放（削除）されたオブジェクト総数（累計）
	size_t m_totalDeleted =0;
};