#pragma once
#include <memory>
#include <functional>
#include "ColliderInfo.h"
#include "Transform.h"

// 前方宣言
class GameObject;

class Collider
{
public:
    Collider();
    virtual ~Collider();

    // 所有者の設定 (Transformアクセス用)
    void SetOwner(const std::weak_ptr<GameObject>& owner);
    // 所有者の取得
    std::shared_ptr<GameObject> GetOwner() const;
    // 所有者のTransformを取得（ショートカット）
    std::shared_ptr<Transform> GetOwnerTransform() const;

    // コライダー種別の取得
    virtual ColliderType GetType() const = 0;

    // AABBの取得（広域判定用）- 毎フレーム更新して返す
    virtual AABB GetAABB() const = 0;

    // デバッグ描画
    virtual void Draw() const {}

    // 衝突イベント (Managerから呼ばれる)
    virtual void OnCollisionEnter(Collider* other);
    virtual void OnCollisionStay(Collider* other);
    virtual void OnCollisionExit(Collider* other);

    // コールバック設定
    // 従来の SetOnCollisionCallback は Enter に割り当てます（互換性維持）
    void SetOnCollisionCallback(std::function<void(Collider*)> callback);
    
    // 詳細なコールバック設定
    void SetOnCollisionEnterCallback(std::function<void(Collider*)> callback);
    void SetOnCollisionStayCallback(std::function<void(Collider*)> callback);
    void SetOnCollisionExitCallback(std::function<void(Collider*)> callback);

    // 有効/無効フラグ
    bool IsActive() const { return m_isActive; }
    void SetActive(bool active) { m_isActive = active; }

    // レイヤー・マスク設定
    void SetLayer(uint32_t layer) { m_info.layer = layer; }
    void SetMask(uint32_t mask) { m_info.mask = mask; }
    uint32_t GetLayer() const { return m_info.layer; }
    uint32_t GetMask() const { return m_info.mask; }

protected:
    std::weak_ptr<GameObject> m_owner; // 所有者への弱参照
    ColliderInfo m_info;               // レイヤー情報等
    bool m_isActive = true;            // 有効フラグ

    // 衝突時コールバック
    std::function<void(Collider*)> m_onCollisionEnter;
    std::function<void(Collider*)> m_onCollisionStay;
    std::function<void(Collider*)> m_onCollisionExit;
};