#include "TriangleBase.h"
#include <cmath> // sin, cos, round を使うため

// コンストラクタ
TriangleBase::TriangleBase(const std::weak_ptr<SceneBase> scene)
	: Primitive(scene)
{
	m_tri.v1 = {0,0,0 };
	m_tri.v2 = {0,0,0 };
	m_tri.v3 = {0,0,0 };
}

// デストラクタ
TriangleBase::~TriangleBase()
{
}

// position を中心（重心）として、中心から size 分の距離に頂点が来る正三角形を生成する。
/// DxLib の座標系では y 正方向が下になる点に注意。
/// angleDeg:0度が右向き（とんがりが右を向く）。角度は DxLib 準拠で時計回りに増加。
void TriangleBase::TriangleBaseInit(const VECTOR& position, float size, float angleDeg)
{
	const double PI =3.14159265358979323846;
	// 正三角形の頂点オフセット（度）: とんがりを baseAngle に合わせるため0,120,240
	const double offsetsDeg[3] = {0.0,120.0,240.0 };

	const double r = static_cast<double>(size);
	double baseAngle = static_cast<double>(angleDeg);

	// 各頂点の計算
	// DxLib の座標系は y が下向きのため、スクリーン上で角度を時計回りに増やすには y に +sin を使う
	m_tri.v1.x = position.x + static_cast<float>(std::cos((baseAngle + offsetsDeg[0]) * PI /180.0) * r);
	m_tri.v1.y = position.y + static_cast<float>(std::sin((baseAngle + offsetsDeg[0]) * PI /180.0) * r);
	m_tri.v1.z = position.z;

	m_tri.v2.x = position.x + static_cast<float>(std::cos((baseAngle + offsetsDeg[1]) * PI /180.0) * r);
	m_tri.v2.y = position.y + static_cast<float>(std::sin((baseAngle + offsetsDeg[1]) * PI /180.0) * r);
	m_tri.v2.z = position.z;

	m_tri.v3.x = position.x + static_cast<float>(std::cos((baseAngle + offsetsDeg[2]) * PI /180.0) * r);
	m_tri.v3.y = position.y + static_cast<float>(std::sin((baseAngle + offsetsDeg[2]) * PI /180.0) * r);
	m_tri.v3.z = position.z;
}

// 保存した頂点で描画する。DrawStoredTriangle は常に現在の裏画面に描画する想定。
void TriangleBase::DrawStoredTriangle(unsigned int color, bool filled) const
{
	// DxLib の DrawTriangle は int 座標を取るため丸める
	int ix1 = static_cast<int>(std::round(m_tri.v1.x));
	int iy1 = static_cast<int>(std::round(m_tri.v1.y));
	int ix2 = static_cast<int>(std::round(m_tri.v2.x));
	int iy2 = static_cast<int>(std::round(m_tri.v2.y));
	int ix3 = static_cast<int>(std::round(m_tri.v3.x));
	int iy3 = static_cast<int>(std::round(m_tri.v3.y));

	if (filled) {
		// 従来どおり塗りつぶし
		DrawTriangle(ix1, iy1, ix2, iy2, ix3, iy3, color, TRUE);
		return;
	}

	// 枠線のみ（単色）。中心はくり抜かれた見た目になる。
	DrawLine(ix1, iy1, ix2, iy2, color);
	DrawLine(ix2, iy2, ix3, iy3, color);
	DrawLine(ix3, iy3, ix1, iy1, color);
}

// 辺単位で色を分けられる描画。
void TriangleBase::DrawStoredTriangleEdges(unsigned int colorEdge1, unsigned int colorEdge2, unsigned int colorEdge3) const
{
	// DxLib の DrawLine は int 座標を取るため丸める
	int ix1 = static_cast<int>(std::round(m_tri.v1.x));
	int iy1 = static_cast<int>(std::round(m_tri.v1.y));
	int ix2 = static_cast<int>(std::round(m_tri.v2.x));
	int iy2 = static_cast<int>(std::round(m_tri.v2.y));
	int ix3 = static_cast<int>(std::round(m_tri.v3.x));
	int iy3 = static_cast<int>(std::round(m_tri.v3.y));

	//直線で各辺を描画
	DrawLine(ix1, iy1, ix2, iy2, colorEdge1);
	DrawLine(ix2, iy2, ix3, iy3, colorEdge2);
	DrawLine(ix3, iy3, ix1, iy1, colorEdge3);
}

// Transform を使って実効サイズを算出して初期化する
void TriangleBase::TriangleBaseInitUsingTransform(float baseSize, float angleDeg)
{
	// ワールド位置を取得
	VECTOR worldPos = m_transform->MatrixToVector();

	// ワールドスケールを合成（X軸スケールを基準にする。必要なら平均や Y を使って調整）
	float worldScaleX =1.0f;
	if (m_transform) {
		worldScaleX = m_transform->GetScale().x;
		// 親を辿って乗算（親のスケールも反映する）
		auto parent = m_transform->GetParentTransform();
		while (parent) {
			worldScaleX *= parent->GetScale().x;
			parent = parent->GetParentTransform();
		}
	}

	float effectiveSize = baseSize * worldScaleX;

	// 初期化ルーチンを呼ぶ
	TriangleBaseInit(worldPos, effectiveSize, angleDeg);
}