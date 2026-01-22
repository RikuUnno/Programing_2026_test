#pragma once
#include "Collider.h"
#include "ObjectInfo.h" 

class CircleCollider : public Collider
{
public:
    CircleCollider();
    virtual ~CircleCollider() = default;

    // 半径の設定 (Local座標系)
    void SetRadius(float radius);
    float GetRadius() const { return m_radius; }

    // 基底クラスの実装
    ColliderType GetType() const override { return ColliderType::Circle; }
    AABB GetAABB() const override;

    // デバッグ描画
    void Draw() const override;

    // ワールド座標系での円情報を取得 (当たり判定計算用)
    Circle GetWorldCircle() const;

private:
    float m_radius = 1.0f;
};