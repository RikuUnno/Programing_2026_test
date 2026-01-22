#include "Collider.h"
#include "ColliderManager.h"
#include "GameObject.h"

Collider::Collider()
{
    // 生成時に自動登録
    ColliderManager::GetInstance().Register(this);
}

Collider::~Collider()
{
    // 破棄時に自動解除
    ColliderManager::GetInstance().Unregister(this);
}

void Collider::SetOwner(const std::weak_ptr<GameObject>& owner)
{
    m_owner = owner;
}

std::shared_ptr<GameObject> Collider::GetOwner() const
{
    return m_owner.lock();
}

std::shared_ptr<Transform> Collider::GetOwnerTransform() const
{
    if (auto ptr = m_owner.lock()) {
        return ptr->GetTransform();
    }
    return nullptr;
}

// 衝突イベントの実装：コールバックを呼び出す
void Collider::OnCollisionEnter(Collider* other)
{
    if (m_onCollisionEnter) m_onCollisionEnter(other);
}

void Collider::OnCollisionStay(Collider* other)
{
    if (m_onCollisionStay) m_onCollisionStay(other);
}

void Collider::OnCollisionExit(Collider* other)
{
    if (m_onCollisionExit) m_onCollisionExit(other);
}

// コールバック設定
void Collider::SetOnCollisionCallback(std::function<void(Collider*)> callback)
{
    // デフォルトは Enter に割り当て（ヒットした瞬間の処理が多いため）
    m_onCollisionEnter = callback;
}

void Collider::SetOnCollisionEnterCallback(std::function<void(Collider*)> callback)
{
    m_onCollisionEnter = callback;
}

void Collider::SetOnCollisionStayCallback(std::function<void(Collider*)> callback)
{
    m_onCollisionStay = callback;
}

void Collider::SetOnCollisionExitCallback(std::function<void(Collider*)> callback)
{
    m_onCollisionExit = callback;
}