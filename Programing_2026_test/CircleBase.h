#pragma once
#include "Primitive.h"
#include "ObjectInfo.h"

class CircleBase
	: public Primitive
{
protected:
	// 継承でしか使わないので protected
	CircleBase(const std::weak_ptr<SceneBase> scene);
	virtual ~CircleBase();

protected:
	// baseRadius（scale=1 のときの半径）を実効サイズに変換して初期化する
	void CircleBaseInitUsingTransform(float baseRadius);

	// 初期化済みの円を描画（保存しておく頂点を描画）
	void DrawStoredCircle(unsigned int color = GetColor(255, 255, 255), bool filled = true) const;

private:
	// 中心 position を中心として radius で円を作る（内部使用）
	void CircleBaseInit(const VECTOR& position, float radius);

private:
	// 保存する円の情報（2D描画用）
	Circle m_circle;
};