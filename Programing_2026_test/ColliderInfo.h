#pragma once
#include <cstdint>

// 軽量AABB（2D前提：x,y）
struct AABB {
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

	// AABB同士の重なり判定
    bool Overlaps(const AABB& other) const {
        return !(maxX < other.minX || other.maxX < minX || maxY < other.minY || other.maxY < minY);
    }
};

// コライダーの種別（派生識別用）
enum class ColliderType : uint32_t {
    Unknown = 0,
    Circle,
    Rect,
    Triangle,   // 追加: 三角形
    Polygon
};

// 広域判定用のコライダー情報
struct ColliderInfo {
    AABB worldAabb;           // グリッド／広域用に毎フレーム更新されるAABB
    uint32_t layer = 0;       // レイヤー（フィルタに使用）
    uint32_t mask = 0xFFFFFFFFu; // 衝突対象マスク
    ColliderType type = ColliderType::Unknown; // 形状タイプ（詳細判定の分岐用）
};

// レイヤー定義
namespace Layer {
    constexpr uint32_t Default      = 0;
    constexpr uint32_t Player       = 1;
    constexpr uint32_t Enemy        = 2;
    constexpr uint32_t PlayerBullet = 3;
    constexpr uint32_t EnemyBullet  = 4; 
}