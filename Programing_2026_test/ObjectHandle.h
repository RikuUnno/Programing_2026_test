#pragma once
#include <cstdint>
#include <limits>
#include <string>
#include <sstream>

struct ObjectHandle
{
	uint32_t index;			// スロットインデックス
	uint32_t generation;	// 世代番号

	ObjectHandle() noexcept : index((std::numeric_limits<uint32_t>::max)()), generation(0) {} // 無効ハンドル
	ObjectHandle(uint32_t idx, uint32_t gen) noexcept : index(idx), generation(gen) {}		// 有効ハンドル

	bool IsValid() const noexcept { return index != (std::numeric_limits<uint32_t>::max)(); } // 有効判定

	// Uint64 変換（ハンドルを 64bit 整数にパック）
	uint64_t ToUint64() const noexcept
	{
		return (static_cast<uint64_t>(generation) << 32) | index;
	}

	// Uint64 変換（64bit 整数からハンドルにアンパック）
	static ObjectHandle FromUint64(uint64_t v) noexcept
	{
		uint32_t idx = static_cast<uint32_t>(v & 0xFFFFFFFFu);
		uint32_t gen = static_cast<uint32_t>(v >> 32);
		return ObjectHandle(idx, gen);
	}

	// 比較演算子
	bool operator==(ObjectHandle const& o) const noexcept { return index == o.index && generation == o.generation; } // 等価比較
	bool operator!=(ObjectHandle const& o) const noexcept { return !(*this == o); }									 // 非等価比較

	// デバッグ用文字列変換
#ifdef _DEBUG
	std::string ToString() const
	{
		std::ostringstream ss;
		ss << "Handle(idx=" << index << ", gen=" << generation << ")";
		return ss.str();
	}
#endif
};