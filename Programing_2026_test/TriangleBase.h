// 三角形のベースクラス
#pragma once
#include "Primitive.h"
#include "DxLib.h"
#include "ObjectInfo.h"
#include <memory>


class TriangleBase
	: public Primitive
{
protected:
	// 継承でしか使わないのでprotected
	TriangleBase(const std::weak_ptr<SceneBase> scene);
	virtual ~TriangleBase();

protected:
	// baseSize（scale=1 のときの半径）を実効サイズに変換して初期化する
	// angleDeg: 回転角（度）。0度が右向き（とんがりが右を向く）。デフォルトは0 度
	void TriangleBaseInitUsingTransform(float baseSize, float angleDeg =0.0f);

	// 初期化済みの頂点を描画（保存しておく頂点を描画）
	void DrawStoredTriangle(unsigned int color = GetColor(255,255,255), bool filled = true) const;

	// 初期化済みの頂点の各辺を個別色で描画（保存しておく頂点を描画）
	void DrawStoredTriangleEdges(unsigned int colorEdge1, unsigned int colorEdge2, unsigned int colorEdge3) const;

private:
	// 中心 position を中心として size（circumradius）で三角形を作る
	// angleDeg: 回転角（度）。0度が右向き（とんがりが右を向く）。デフォルトは0 度
	void TriangleBaseInit(const VECTOR& position, float size, float angleDeg =0.0f);

private:
	// 保存する頂点（2D描画用）
	Triangle m_tri;
};