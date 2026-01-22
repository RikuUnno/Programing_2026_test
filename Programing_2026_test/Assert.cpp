#include "Assert.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

void Assert::Fail(const char* message, const char* file, int line, const char* function) noexcept
{
	try
	{
		std::string out;
		out += "Assertion failed: ";
		out += (message ? message : "");
		out += "\nFile: ";
		out += (file ? file : "");
		out += "\nLine: ";
		out += std::to_string(line);
		out += "\nFunction: ";
		out += (function ? function : "");

#ifdef _WIN32
		// デバッグ用にダイアログとデバッガ出力
		OutputDebugStringA(out.c_str());
		MessageBoxA(nullptr, out.c_str(), "Assertion Failed", MB_ICONERROR | MB_OK);
#else
		std::cerr << out << std::endl;
#endif
	}
	catch (...)
	{
		// 何が起きても abort へ
	}

	// 開発中はここでプロセスを止める
	std::abort();
}

void Assert::Check(bool condition, const char* message, const char* file, int line, const char* function) noexcept
{
	if (!condition)
		Fail(message, file, line, function);
}