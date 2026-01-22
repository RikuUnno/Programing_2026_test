#include "TriangleCollider.h"
#include <algorithm>
#include <cmath>
#include "DxLib.h"

TriangleCollider::TriangleCollider()
{
    // デフォルトで初期化
    m_localV1 = VGet(0, -10, 0);
    m_localV2 = VGet(-10, 10, 0);
    m_localV3 = VGet(10, 10, 0);
}

void TriangleCollider::SetLocalVertices(const VECTOR& v1, const VECTOR& v2, const VECTOR& v3)
{
    m_localV1 = v1;
    m_localV2 = v2;
    m_localV3 = v3;
}

void TriangleCollider::SetupFromBase(float size, float angleDeg)
{
    // TriangleBaseInitのロジックを参考にローカル頂点を設定
    // 0度で上向き(0,-1,0)を基準とする
    
    // ラジアン変換
    float rad = angleDeg * (DX_PI_F / 180.0f);
    float angle1 = rad + (DX_PI_F * 1.5f); // 上
    float angle2 = rad + (DX_PI_F * 1.5f) + (DX_PI_F * 2.0f / 3.0f); // 右下へ
    float angle3 = rad + (DX_PI_F * 1.5f) + (DX_PI_F * 4.0f / 3.0f); // 左下へ

    m_localV1.x = std::cos(angle1) * size;
    m_localV1.y = std::sin(angle1) * size;
    m_localV1.z = 0.0f;

    m_localV2.x = std::cos(angle2) * size;
    m_localV2.y = std::sin(angle2) * size;
    m_localV2.z = 0.0f;

    m_localV3.x = std::cos(angle3) * size;
    m_localV3.y = std::sin(angle3) * size;
    m_localV3.z = 0.0f;
}

AABB TriangleCollider::GetAABB() const
{
    Triangle worldTri = GetWorldTriangle();
    
    AABB aabb;
    // Windowsマクロとの競合を防ぐため (std::min), (std::max) と記述
    aabb.minX = (std::min)({ worldTri.v1.x, worldTri.v2.x, worldTri.v3.x });
    aabb.maxX = (std::max)({ worldTri.v1.x, worldTri.v2.x, worldTri.v3.x });
    aabb.minY = (std::min)({ worldTri.v1.y, worldTri.v2.y, worldTri.v3.y });
    aabb.maxY = (std::max)({ worldTri.v1.y, worldTri.v2.y, worldTri.v3.y });
    
    return aabb;
}

// デバッグ描画実装
void TriangleCollider::Draw() const
{
    Triangle t = GetWorldTriangle();
    // 緑色の枠線で表示
    DrawTriangle(
        static_cast<int>(t.v1.x), static_cast<int>(t.v1.y),
        static_cast<int>(t.v2.x), static_cast<int>(t.v2.y),
        static_cast<int>(t.v3.x), static_cast<int>(t.v3.y),
        GetColor(0, 255, 0), FALSE // FALSE=塗りつぶさない
    );
}

Triangle TriangleCollider::GetWorldTriangle() const
{
    Triangle t;
    // ワールド行列を取得して変換する
    auto transform = GetOwnerTransform();
    if (transform)
    {
        // Transformの現在の行列を使ってローカル頂点をワールドへ変換
        // Transform::GetWorldMatrix() は Update されている前提
        MATRIX mat = transform->GetWorldMatrix();
        
        t.v1 = VTransform(m_localV1, mat);
        t.v2 = VTransform(m_localV2, mat);
        t.v3 = VTransform(m_localV3, mat);
    }
    else
    {
        // 親がなければそのまま
        t.v1 = m_localV1;
        t.v2 = m_localV2;
        t.v3 = m_localV3;
    }
    return t;
}