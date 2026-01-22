#include "CircleBase.h"
#include <cmath>
#include "Transform.h"
#include "DxLib.h"

// コンストラクタ
CircleBase::CircleBase(const std::weak_ptr<SceneBase> scene)
	: Primitive(scene)
{
	m_circle.center = { 0.0f, 0.0f, 0.0f };
	m_circle.radius = 0.0f;
}

// デストラクタ
CircleBase::~CircleBase()
{
}

// Transform を使ってワールド位置・ワールドスケールを算出し、
// baseRadius（scale=1）から実効半径を求めて初期化する
void CircleBase::CircleBaseInitUsingTransform(float baseRadius)
{
	// ワールド位置（Transform が持つワールド座標取得メソッドを使用）
	VECTOR worldPos = m_transform->MatrixToVector();

	// 親チェインを辿ってワールドスケールを合成する
	float worldScaleX = 1.0f;
	float worldScaleY = 1.0f;
	// 自身のスケールを乗算
	{
		VECTOR s = m_transform->GetScale();
		worldScaleX *= s.x;
		worldScaleY *= s.y;
	}
	// 親のスケールを乗算
	auto parent = m_transform->GetParentTransform();
	while (parent)
	{
		VECTOR ps = parent->GetScale();
		worldScaleX *= ps.x;
		worldScaleY *= ps.y;
		parent = parent->GetParentTransform();
	}

	// 実効半径は X,Y の平均スケールを適用（非等方スケールの簡易処理）
	float effectiveScale = (worldScaleX + worldScaleY) * 0.5f;
	float effectiveRadius = baseRadius * effectiveScale;

	CircleBaseInit(worldPos, effectiveRadius);
}

// 中心 position を中心として radius で円を設定する（内部で保存）
void CircleBase::CircleBaseInit(const VECTOR& position, float radius)
{
	m_circle.center = position;
	m_circle.radius = radius;
}

// 保存した円を描画する
void CircleBase::DrawStoredCircle(unsigned int color, bool filled) const
{
	// DxLib は int 座標をとるので丸める
	int ix = static_cast<int>(std::round(m_circle.center.x));
	int iy = static_cast<int>(std::round(m_circle.center.y));
	int ir = static_cast<int>(std::round(m_circle.radius));

	// DrawCircle(x,y,r,color,fillFlag) : fillFlag TRUE -> 塗り, FALSE -> 枠のみ
	DrawCircle(ix, iy, ir, color, filled ? TRUE : FALSE);
}



