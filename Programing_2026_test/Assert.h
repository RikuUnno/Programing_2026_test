#pragma once
#include <string>

class Assert
{
public:
	// 強制失敗：メッセージ・ファイル・行・関数名を受け取り報告して abort する
	static void Fail(const char* message, const char* file, int line, const char* function) noexcept;

	// 条件チェックのヘルパー（マクロから使う用）
	static void Check(bool condition, const char* message, const char* file, int line, const char* function) noexcept;
};

// マクロ定義（デバッグ時のみ有効）
#ifndef NDEBUG
	#define ASSERT_MSG(cond, msg) \
		((cond) ? (void)0 : Assert::Fail((msg), __FILE__, __LINE__, __func__))

	#define ASSERT(cond) \
		((cond) ? (void)0 : Assert::Fail(#cond, __FILE__, __LINE__, __func__))

	// 条件チェックを明示的に呼ぶ場合
	#define ASSERT_CHECK(cond, msg) \
		(Assert::Check((cond), (msg), __FILE__, __LINE__, __func__))

	// 条件に関係なく失敗させたい場合
	#define ASSERT_FAIL(msg) \
		(Assert::Fail((msg), __FILE__, __LINE__, __func__))
#else
	// リリースでは無効化（副作用のある式を渡すときは注意）
	#define ASSERT_MSG(cond, msg) ((void)0)
	#define ASSERT(cond) ((void)0)
	#define ASSERT_CHECK(cond, msg) ((void)0)
	#define ASSERT_FAIL(msg) ((void)0)
#endif