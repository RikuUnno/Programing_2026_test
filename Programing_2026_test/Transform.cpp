#include "Transform.h"

Transform::Transform()
{
	m_position = VGet(0.0f, 0.0f, 0.0f); // 初期位置は原点
	m_rotation = VGet(0.0f, 0.0f, 0.0f); // 初期回転はなし
	m_scale = VGet(1.0f, 1.0f, 1.0f);	 // 初期スケールは1倍

	m_positionMatrix = MGetIdent(); // 単位行列で初期化
	m_rotationMatrix = MGetIdent(); // 単位行列で初期化
	m_scaleMatrix = MGetIdent();    // 単位行列で初期化

	m_localMatrix = MGetIdent();	// 単位行列で初期化
	m_worldMatrix = MGetIdent();	// 単位行列で初期化
}

Transform::Transform(VECTOR position, VECTOR rotation, VECTOR scale)
{
	m_position = position;
	m_rotation = rotation;
	m_scale = scale;

	m_positionMatrix = MGetIdent();
	m_rotationMatrix = MGetIdent();
	m_scaleMatrix = MGetIdent();

	m_localMatrix = MGetIdent();
	m_worldMatrix = MGetIdent();
}

Transform::~Transform()
{}

void Transform::UpdateMatrix()
{
	// 各行列の更新
	UpdateScaleMatrix();
	UpdateRotationMatrix();
	UpdatePositionMatrix();

	// ローカル行列の更新
	m_localMatrix = MMult(MMult(m_scaleMatrix, m_rotationMatrix), m_positionMatrix); // 拡縮→回転→移動の順で行列を掛け合わせる
}

void Transform::UpdatePositionMatrix()
{
	m_positionMatrix = MGetTranslate(m_position); // 移動行列の取得
}

void Transform::UpdateRotationMatrix()
{
	// DxLib基準 (X → Y → Z)
	MATRIX m_rotX = MGetRotX(m_rotation.x);
	MATRIX m_rotY = MGetRotY(m_rotation.y);
	MATRIX m_rotZ = MGetRotZ(m_rotation.z);

	m_rotationMatrix = MMult(m_rotZ, MMult(m_rotY, m_rotX)); // 回転行列の取得
}

void Transform::UpdateScaleMatrix()
{
	m_scaleMatrix = MGetScale(m_scale); // 拡縮行列の取得
}

// ローカル→ワールド変換行列の取得
void Transform::LocalToWorldMatrix()
{
	if(m_parent.expired())
	{
		// 親がいない場合はローカル行列をそのままワールド行列に設定
		m_worldMatrix = m_localMatrix;
	}
	else
	{
		// 親がいる場合は親のワールド行列と掛け合わせる
		auto parentShared = m_parent.lock();
		if (parentShared)
		{
			m_worldMatrix = MMult(parentShared->GetWorldMatrix(), m_localMatrix);
		}
		else
		{
			// 親がロックできなかった場合はローカル行列をそのままワールド行列に設定
			m_worldMatrix = m_localMatrix;
		}
	}
}

float Transform::RadToDeg(float& num) // ラジアンを度に変換
{
	const float radToDeg = 180.0f / 3.14159265f;
	num *= radToDeg;
	return num;
}

float Transform::DegToRad(float& num) // 度をラジアンに変換
{
	const float degToRad = 3.14159265f / 180.0f;
	num *= degToRad;
	return num;
}

VECTOR Transform::MatrixToVector()
{
	return VTransform((VGet(0.0f, 0.0f, 0.0f)), m_worldMatrix);
}

void Transform::SetParent(std::weak_ptr<Transform> parent)
{
	m_parent = parent;
}

void Transform::ClearParent()
{
	m_parent.reset();
}