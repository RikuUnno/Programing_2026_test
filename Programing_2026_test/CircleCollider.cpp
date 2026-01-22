#include "CircleCollider.h"
#include <algorithm>
#include <cmath>
#include "DxLib.h"

CircleCollider::CircleCollider()
{
}

void CircleCollider::SetRadius(float radius)
{
    m_radius = radius;
}

AABB CircleCollider::GetAABB() const
{
    Circle worldCircle = GetWorldCircle();
    AABB aabb;
    aabb.minX = worldCircle.center.x - worldCircle.radius;
    aabb.maxX = worldCircle.center.x + worldCircle.radius;
    aabb.minY = worldCircle.center.y - worldCircle.radius;
    aabb.maxY = worldCircle.center.y + worldCircle.radius;
    return aabb;
}

// デバッグ描画実装
void CircleCollider::Draw() const
{
    Circle c = GetWorldCircle();
    // 緑色の枠線で表示
    DrawCircle(static_cast<int>(c.center.x), static_cast<int>(c.center.y), static_cast<int>(c.radius), GetColor(0, 255, 0), FALSE);
}

Circle CircleCollider::GetWorldCircle() const
{
    Circle c;
    c.center = VGet(0, 0, 0);
    c.radius = m_radius;

    auto t = GetOwnerTransform();
    if (t) {
        // ワールド座標
        c.center = t->GetPosition();

        // スケールを考慮 (XとYの大きい方を採用して円を保つ簡易実装)
        VECTOR scale = t->GetScale();
        // Windowsマクロとの競合を防ぐため (std::max) と記述
        float maxScale = (std::max)(std::abs(scale.x), std::abs(scale.y));
        c.radius = m_radius * maxScale;
    }
    return c;
}