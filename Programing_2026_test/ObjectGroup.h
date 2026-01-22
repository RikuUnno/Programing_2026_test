#pragma once
#include "ObjectHandle.h"
#include <vector>
#include <algorithm>
#include <functional>
#include <mutex>
#include <memory>

// Forward declarations to avoid circular includes
class ObjectManager;
class GameObject;

class ObjectGroup
{
public:
	// ハンドルを追加（無効ハンドルは無視）
	void Add(ObjectHandle h);
	// 引数: ハンドル

	// ハンドルを削除（存在すれば）
	void Remove(ObjectHandle h);
	// 引数: ハンドル

	// 全クリア
	void Clear();
	// デストラクタで呼ぶなど

	// 全オブジェクトを Update（無効ハンドルはリストから削除）
	void UpdateAll();
	// Scene等のUpdateに配置

	// 全オブジェクトを Draw（無効ハンドルはリストから削除）
	void DrawAll();
	// 描画処理に配置

	// 全オブジェクトを End（無効ハンドルはリストから削除）
	void EndAll();
	// シーン終了時などに配置

	// 任意の操作を各オブジェクトに対して行うヘルパー
	void ForEach(const std::function<void(std::shared_ptr<GameObject>)>& func);
	// 引数: 各オブジェクトに対して行う関数

	// 子オブジェクトを削除
	void RemoveAllChild();
	// 親オブジェクトのEndなどで呼ぶ
	// RemoveAllChild→ EndAll→ Clear の順で呼ぶこと

private:
	std::vector<ObjectHandle> m_handles; // オブジェクトハンドルのリスト
	std::mutex m_mutex;
};