#pragma once
#include <memory>
#include <type_traits>
#include "GameObject.h"
#include "Assert.h"

class Factory
{
public:
	// インスタンス取得
	static Factory& GetInstance();

private:
	Factory() = default;
	~Factory() = default;
	Factory(const Factory&) = delete;
	Factory& operator=(const Factory&) = delete;

public:
	// CreateObject: オブジェクトを生成して返す（初期化は呼び出し側で行う）
	template <typename T, typename... Args>
	std::shared_ptr<T> CreateObject(Args&&... args)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");

		// make_sharedで生成（初期化は呼び出し側で行う）
		auto obj = std::make_shared<T>(std::forward<Args>(args)...);

		return obj;
	}
};