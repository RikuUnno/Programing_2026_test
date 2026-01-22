#include "ObjectPool.h"
#include <algorithm>

// 非テンプレート実装をここへ移動

std::shared_ptr<GameObject> ObjectPool::Get(ObjectHandle handle)
{
	if (m_shuttingDown.load(std::memory_order_acquire)) {
		return nullptr;
	}
	ObjectPool::ActivityGuard ag(m_activeOps);

	std::lock_guard<std::mutex> lk(m_mutex);

	if (!IsHandleValid_NoLock(handle)) return nullptr;
	Slot& slot = m_slots[handle.index];
	slot.lastUsed = std::chrono::steady_clock::now();
	return slot.obj;
}

bool ObjectPool::Release(ObjectHandle handle)
{
	// シャットダウン中はリリースを拒否
	if (m_shuttingDown.load(std::memory_order_acquire)) {
		DebugLogFmt("[ObjectPool] Release refused: shutting down idx=%u gen=%u\n", handle.index, handle.generation);
		return false;
	}
	ObjectPool::ActivityGuard ag(m_activeOps); // アクティブ操作ガード

	std::lock_guard<std::mutex> lk(m_mutex); // ロック取得

	// 有効性チェック
	if (!IsHandleValid_NoLock(handle)) {
		DebugLogFmt("[ObjectPool] Release INVALID handle idx=%u gen=%u\n", handle.index, handle.generation);
		return false;
	}

	// スロットを解放状態にする
	Slot& slot = m_slots[handle.index];
	slot.inUse = false;
	slot.lastUsed = std::chrono::steady_clock::now();
	++slot.generation;
	// freeIndices に重複して入らないようにチェックしてから追加
	if (std::find(m_freeIndices.begin(), m_freeIndices.end(), handle.index) == m_freeIndices.end()) {
		m_freeIndices.push_back(handle.index);
		DebugLogFmt("[ObjectPool] Release idx=%u newGen=%u freeCount=%zu\n", handle.index, slot.generation, m_freeIndices.size());
	}
	else {
		DebugLogFmt("[ObjectPool] Release idx=%u already free (gen=%u)\n", handle.index, slot.generation);
	}
	return true;
}

void ObjectPool::ClearAll()
{
	// シャットダウン開始を宣言
	m_shuttingDown.store(true, std::memory_order_release);

	// アクティブ操作が終わるのを待つ（短時間のスピン）
	const int maxWaitMs = 5000;
	int waited = 0;
	while (m_activeOps.load(std::memory_order_acquire) != 0 && waited < maxWaitMs) {
		Sleep(1);
		++waited;
	}
	if (m_activeOps.load(std::memory_order_acquire) != 0) {
		DebugLogFmt("[ObjectPool] ClearAll: warning activeOps=%d after wait\n", m_activeOps.load());
	}

	// クリア処理
	std::lock_guard<std::mutex> lk(m_mutex);

	for (auto& s : m_slots) // 全スロットを走査
	{
		// 実際にオブジェクトが存在していたら削除カウントを増やす
		if (s.obj) {
			++m_totalDeleted;
		}
		s.obj.reset();
		s.inUse = false;
		++s.generation;
		s.lastUsed = std::chrono::steady_clock::now();
	}

	m_freeIndices.clear();

	for (uint32_t i = 0; i < m_slots.size(); ++i) m_freeIndices.push_back(i);
}

size_t ObjectPool::CleanupIdle(double maxIdleSeconds)
{
	if (m_shuttingDown.load(std::memory_order_acquire)) {
		return 0;
	}
	ObjectPool::ActivityGuard ag(m_activeOps);

	using Clock = std::chrono::steady_clock;
	auto now = Clock::now();

	// 破棄はロック外で行うための収集リスト
	std::vector<std::shared_ptr<GameObject>> toDestroy;
	size_t removed = 0;

	{
		std::lock_guard<std::mutex> lk(m_mutex);

		for (uint32_t i = 0; i < m_slots.size(); ++i)
		{
			Slot& slot = m_slots[i];
			if (slot.inUse) continue;				// 使用中はスキップ
			if (!slot.obj) continue;				// オブジェクトが存在しない場合はスキップ
			if (slot.obj.use_count() > 1) continue;	// 外部参照がある場合はスキップ

			// アイドル時間計算
			auto idle = std::chrono::duration_cast<std::chrono::duration<double>>(now - slot.lastUsed).count();

			// 指定秒数以上アイドルなら解放候補として抜き出す（ここでは actual reset は行わない）
			if (idle >= maxIdleSeconds)
			{
				++m_totalDeleted;               // 削除カウントはロック内で増やす
				toDestroy.push_back(std::move(slot.obj)); // slot.obj をムーブしてロック外で破棄
				++slot.generation;
				// freeIndices に入っていなければ追加（重複排除）
				if (std::find(m_freeIndices.begin(), m_freeIndices.end(), i) == m_freeIndices.end())
				{
					m_freeIndices.push_back(i); // 空きリストに追加
				}
				++removed;
				// slot.obj はムーブで nullptr になっている
			}
		}
	} // lk 解放

	// ここで toDestroy の shared_ptr をスコープで破棄すると実際のデストラクタが走る（ロックなし）
	toDestroy.clear();

	return removed;
}

bool ObjectPool::IsHandleValid(ObjectHandle handle)
{
	// allow during shutdown (reads only)
	std::lock_guard<std::mutex> lk(m_mutex);
	return IsHandleValid_NoLock(handle);
}

size_t ObjectPool::PoolSize() const
{
	std::lock_guard<std::mutex> lk(m_mutex);
	return m_slots.size();
}

size_t ObjectPool::ActiveCount() const
{
	std::lock_guard<std::mutex> lk(m_mutex);
	size_t cnt = 0;
	for (const auto& s : m_slots) if (s.inUse && s.obj) ++cnt;
	return cnt;
}

size_t ObjectPool::FreeCount() const {
	std::lock_guard<std::mutex> lk(m_mutex);
	return m_freeIndices.size();
}

void ObjectPool::DumpState() const {
	std::lock_guard<std::mutex> lk(m_mutex);
	DebugLogFmt("Pool: slots=%zu free=%zu\n", m_slots.size(), m_freeIndices.size());
	for (uint32_t i = 0; i < m_slots.size(); ++i) {
		const Slot& s = m_slots[i];
		int hasObj = s.obj ? 1 : 0;
		int inUse = s.inUse ? 1 : 0;
		DebugLogFmt(" slot[%u] gen=%u inUse=%d hasObj=%d use_count=%d\n",
			i, s.generation, inUse, hasObj, s.obj ? static_cast<int>(s.obj.use_count()) : 0);
	}
}

size_t ObjectPool::TotalCreated() const {
	std::lock_guard<std::mutex> lk(m_mutex);
	return m_totalCreated;
}

size_t ObjectPool::TotalDeleted() const {
	std::lock_guard<std::mutex> lk(m_mutex);
	return m_totalDeleted;
}

bool ObjectPool::IsHandleValid_NoLock(ObjectHandle handle) const
{
	if (!handle.IsValid()) return false;
	if (handle.index >= m_slots.size()) return false;
	const Slot& slot = m_slots[handle.index];
	if (!slot.inUse) return false;
	if (slot.generation != handle.generation) return false;
	if (!slot.obj) return false;
	return true;
}

void ObjectPool::UpdateAllObjectsScene(const std::weak_ptr<SceneBase>& scene)
{
	std::lock_guard<std::mutex> lk(m_mutex);
	for (auto& s : m_slots) {
		if (s.obj) {
			s.obj->SetScene(scene);
		}
	}
}