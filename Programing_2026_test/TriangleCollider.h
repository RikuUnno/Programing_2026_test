#pragma once
#include "Collider.h"
#include "ObjectInfo.h"

// 三角形コライダー
class TriangleCollider : public Collider
{
public:
    TriangleCollider();
    virtual ~TriangleCollider() = default;

    // トライアングルの形状設定 (ローカル座標)
    // 頂点オフセットを直接指定
    void SetLocalVertices(const VECTOR& v1, const VECTOR& v2, const VECTOR& v3);
    
    // 正三角形など、サイズと角度から形状を設定するヘルパー
    // (TriangleBaseInitUsingTransformと同様のロジックで設定する場合に使用)
    void SetupFromBase(float size, float angleDeg = 0.0f);

    // 基底クラスの実装
    ColliderType GetType() const override { return ColliderType::Triangle; }
    AABB GetAABB() const override;

    // デバッグ描画
    void Draw() const override;

    // ワールド座標系での三角形情報を取得 (当たり判定計算用)
    Triangle GetWorldTriangle() const;

private:
    // ローカル座標での3頂点
    VECTOR m_localV1;
    VECTOR m_localV2;
    VECTOR m_localV3;
};