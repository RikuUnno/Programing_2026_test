#pragma once
#include <vector>
#include <memory>
#include <set>      // 追加
#include <utility>  // 追加 for std::pair
#include "Collider.h"

// 前方宣言
class CircleCollider;
class TriangleCollider;

class ColliderManager
{
public:
    static ColliderManager& GetInstance();

    // コライダーの登録・解除
    void Register(Collider* collider);
    void Unregister(Collider* collider);

    // 毎フレーム呼ぶ：当たり判定の実行
    void Execute();

    // デバッグ描画
    void DrawDebug() const;

private:
    ColliderManager() = default;
    ~ColliderManager() = default;
    ColliderManager(const ColliderManager&) = delete;
    ColliderManager& operator=(const ColliderManager&) = delete;

    // 2つのコライダー間の判定ロジック
    bool CheckCollision(Collider* a, Collider* b);
    
    // 詳細判定
    bool CheckCircleCircle(CircleCollider* a, CircleCollider* b);
    bool CheckTriangleTriangle(TriangleCollider* a, TriangleCollider* b);
    bool CheckCircleTriangle(CircleCollider* c, TriangleCollider* t);

    // 補助関数
    float GetPointLineDistSq(const VECTOR& p, const VECTOR& a, const VECTOR& b);

private:
    std::vector<Collider*> m_colliders; // 管理リスト

    // ★追加: 前フレームの衝突ペア (Colliderポインタのペア。常に first < second で保存)
    std::set<std::pair<Collider*, Collider*>> m_prevCollisions;
};