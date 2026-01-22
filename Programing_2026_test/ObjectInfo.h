#pragma once
#include "DxLib.h"

// 基本的な形状情報構造体群(2D用)

// 円情報
struct Circle
{
	VECTOR center;
	float radius;
};

// 三角形情報
struct Triangle
{
	VECTOR v1;
	VECTOR v2;
	VECTOR v3;
};

// etc...
// 四角形情報(オブジェクト情報)
// カプセル情報(オブジェクト情報)

