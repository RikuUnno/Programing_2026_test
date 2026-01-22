#include "ColliderManager.h"
#include "CircleCollider.h"
#include "TriangleCollider.h"
#include "DxLib.h"
#include "SceneBase.h" 
#include "GameObject.h" 
#include <algorithm>
#include <set>

ColliderManager& ColliderManager::GetInstance()
{
    static ColliderManager instance;
    return instance;
}

void ColliderManager::Register(Collider* collider)
{
    if (!collider) return;
    m_colliders.push_back(collider);
}

void ColliderManager::Unregister(Collider* collider)
{
    if (!collider) return;

    // リストから削除
    auto it = std::find(m_colliders.begin(), m_colliders.end(), collider);
    if (it != m_colliders.end()) {
        m_colliders.erase(it);
    }

    // 削除されるコライダーに関連する衝突情報をクリーンアップ
    // これを行わないと、相手側に Exit が呼ばれない、あるいはダングリングポインタが残る
    for (auto itCollision = m_prevCollisions.begin(); itCollision != m_prevCollisions.end(); )
    {
        Collider* c1 = itCollision->first;
        Collider* c2 = itCollision->second;

        if (c1 == collider || c2 == collider)
        {
            // 相手側を取得
            Collider* other = (c1 == collider) ? c2 : c1;
            
            // 相手側に Exit を通知 (相手が生きていれば)
            // ※削除される collider 自身には通知しない（デストラクタ中などの可能性があるため）
            if (other) {
                other->OnCollisionExit(collider);
            }

            // 履歴から削除
            itCollision = m_prevCollisions.erase(itCollision);
        }
        else
        {
            ++itCollision;
        }
    }
}

void ColliderManager::Execute()
{
    // 現在のシーンを取得
    auto currentScene = SceneBase::GetCurrentSceneWeak().lock();
    if (!currentScene) return; // シーンが無ければ判定しない

    size_t count = m_colliders.size();
    
    // 現在のフレームでの衝突ペアを保存するセット
    std::set<std::pair<Collider*, Collider*>> currentCollisions;

    // 1. 総当たり判定 & 現在の衝突リスト作成
    for (size_t i = 0; i < count; ++i)
    {
        Collider* colA = m_colliders[i];
        if (!colA->IsActive()) continue;

        // colA のシーンチェック
        // オーナーが無効、または別シーンのオブジェクトならスキップ
        auto ownerA = colA->GetOwner();
        if (!ownerA || ownerA->GetSceneWeak().lock() != currentScene) continue;


        for (size_t j = i + 1; j < count; ++j)
        {
            Collider* colB = m_colliders[j];
            if (!colB->IsActive()) continue;

            // colB のシーンチェック
            // 異なるシーン同士の判定は行わない（通常ありえないが念のため)
            auto ownerB = colB->GetOwner();
            if (!ownerB || ownerB->GetSceneWeak().lock() != currentScene) continue;


            // マスク判定
            bool matchA = (colA->GetMask() & (1 << colB->GetLayer()));
            bool matchB = (colB->GetMask() & (1 << colA->GetLayer()));
            if (!matchA && !matchB) continue;

            // AABB判定
            if (!colA->GetAABB().Overlaps(colB->GetAABB())) continue;

            // 詳細判定
            if (CheckCollision(colA, colB))
            {
                // ポインタのアドレス順でペアを作成 (重複防止)
                Collider* first = (colA < colB) ? colA : colB;
                Collider* second = (colA < colB) ? colB : colA;
                
                currentCollisions.insert(std::make_pair(first, second));
            }
        }
    }

    // 2. Enter / Stay イベントの発行
    for (const auto& pair : currentCollisions)
    {
        Collider* c1 = pair.first;
        Collider* c2 = pair.second;

        // 前フレームにも存在したか？
        if (m_prevCollisions.find(pair) != m_prevCollisions.end())
        {
            // Stay
            c1->OnCollisionStay(c2);
            c2->OnCollisionStay(c1);
        }
        else
        {
            // Enter (新規衝突)
            c1->OnCollisionEnter(c2);
            c2->OnCollisionEnter(c1);
        }
    }

    // 3. Exit イベントの発行 (前フレームにあったが今はないもの)
    for (const auto& pair : m_prevCollisions)
    {
        if (currentCollisions.find(pair) == currentCollisions.end())
        {
            // Exit (離れた)
            pair.first->OnCollisionExit(pair.second);
            pair.second->OnCollisionExit(pair.first);
        }
    }

    // 履歴更新
    m_prevCollisions = currentCollisions;
}

bool ColliderManager::CheckCollision(Collider* a, Collider* b)
{
    ColliderType typeA = a->GetType();
    ColliderType typeB = b->GetType();

    // Circle vs Circle
    if (typeA == ColliderType::Circle && typeB == ColliderType::Circle) {
        return CheckCircleCircle(static_cast<CircleCollider*>(a), static_cast<CircleCollider*>(b));
    }
    // Triangle vs Triangle
    else if (typeA == ColliderType::Triangle && typeB == ColliderType::Triangle) {
        return CheckTriangleTriangle(static_cast<TriangleCollider*>(a), static_cast<TriangleCollider*>(b));
    }
    // Circle vs Triangle
    else if (typeA == ColliderType::Circle && typeB == ColliderType::Triangle) {
        return CheckCircleTriangle(static_cast<CircleCollider*>(a), static_cast<TriangleCollider*>(b));
    }
    // Triangle vs Circle
    else if (typeA == ColliderType::Triangle && typeB == ColliderType::Circle) {
        return CheckCircleTriangle(static_cast<CircleCollider*>(b), static_cast<TriangleCollider*>(a));
    }

    return false;
}

bool ColliderManager::CheckCircleCircle(CircleCollider* a, CircleCollider* b)
{
    Circle c1 = a->GetWorldCircle();
    Circle c2 = b->GetWorldCircle();

    float rSum = c1.radius + c2.radius;
    float dx = c1.center.x - c2.center.x;
    float dy = c1.center.y - c2.center.y;
    
    return (dx * dx + dy * dy) <= (rSum * rSum);
}

// 補助関数
VECTOR SubV(const VECTOR& a, const VECTOR& b) {
    return VGet(a.x - b.x, a.y - b.y, 0);
}

bool SegmentSegmentIntersect(const VECTOR& p1, const VECTOR& p2, const VECTOR& q1, const VECTOR& q2)
{
    auto Cross = [](VECTOR a, VECTOR b) { return a.x * b.y - a.y * b.x; };
    VECTOR r = SubV(p2, p1);
    VECTOR s = SubV(q2, q1);
    VECTOR qp = SubV(q1, p1);

    float rxs = Cross(r, s);
    if (rxs == 0) return false;

    float t = Cross(qp, s) / rxs;
    float u = Cross(qp, r) / rxs;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

bool IsPointInTriangle(const VECTOR& p, const Triangle& tri)
{
    auto Cross = [](VECTOR a, VECTOR b) { return a.x * b.y - a.y * b.x; };
    VECTOR ab = SubV(tri.v2, tri.v1);
    VECTOR bc = SubV(tri.v3, tri.v2);
    VECTOR ca = SubV(tri.v1, tri.v3);

    VECTOR ap = SubV(p, tri.v1);
    VECTOR bp = SubV(p, tri.v2);
    VECTOR cp = SubV(p, tri.v3);

    float c1 = Cross(ab, ap);
    float c2 = Cross(bc, bp);
    float c3 = Cross(ca, cp);

    return (c1 >= 0 && c2 >= 0 && c3 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0);
}

bool ColliderManager::CheckTriangleTriangle(TriangleCollider* a, TriangleCollider* b)
{
    Triangle t1 = a->GetWorldTriangle();
    Triangle t2 = b->GetWorldTriangle();

    if (IsPointInTriangle(t1.v1, t2) || IsPointInTriangle(t1.v2, t2) || IsPointInTriangle(t1.v3, t2)) return true;
    if (IsPointInTriangle(t2.v1, t1) || IsPointInTriangle(t2.v2, t1) || IsPointInTriangle(t2.v3, t1)) return true;

    VECTOR edges1[3][2] = { {t1.v1, t1.v2}, {t1.v2, t1.v3}, {t1.v3, t1.v1} };
    VECTOR edges2[3][2] = { {t2.v1, t2.v2}, {t2.v2, t2.v3}, {t2.v3, t2.v1} };

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (SegmentSegmentIntersect(edges1[i][0], edges1[i][1], edges2[j][0], edges2[j][1])) {
                return true;
            }
        }
    }
    return false;
}

float ColliderManager::GetPointLineDistSq(const VECTOR& p, const VECTOR& a, const VECTOR& b)
{
    VECTOR ab = SubV(b, a);
    VECTOR ap = SubV(p, a);

    float lenSq = ab.x * ab.x + ab.y * ab.y;
    if (lenSq == 0.0f) {
        return ap.x * ap.x + ap.y * ap.y;
    }

    float t = (ap.x * ab.x + ap.y * ab.y) / lenSq;
    t = (std::max)(0.0f, (std::min)(1.0f, t));

    VECTOR closest = VGet(a.x + ab.x * t, a.y + ab.y * t, 0);
    VECTOR diff = SubV(p, closest);

    return diff.x * diff.x + diff.y * diff.y;
}

bool ColliderManager::CheckCircleTriangle(CircleCollider* c, TriangleCollider* t)
{
    Circle circle = c->GetWorldCircle();
    Triangle tri = t->GetWorldTriangle();

    if (IsPointInTriangle(circle.center, tri)) return true;

    float rSq = circle.radius * circle.radius;
    if (GetPointLineDistSq(circle.center, tri.v1, tri.v2) <= rSq) return true;
    if (GetPointLineDistSq(circle.center, tri.v2, tri.v3) <= rSq) return true;
    if (GetPointLineDistSq(circle.center, tri.v3, tri.v1) <= rSq) return true;

    return false;
}

void ColliderManager::DrawDebug() const
{
#ifdef _DEBUG
    // デバッグ描画もシーンチェック
    auto currentScene = SceneBase::GetCurrentSceneWeak().lock();
    if (!currentScene) return;

    for (auto* col : m_colliders) {
        if (!col->IsActive()) continue;
        auto owner = col->GetOwner();
        if (!owner || owner->GetSceneWeak().lock() != currentScene) continue;

        col->Draw();
    }
#endif // _DEBUG
}