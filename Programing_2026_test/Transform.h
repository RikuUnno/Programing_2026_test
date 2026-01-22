#pragma once
#include "DxLib.h"
#include <memory>

class Transform
{
private:
	std::weak_ptr<Transform> m_parent; // 親Transformのポインタ

private:
	VECTOR m_position;			// 移動
	VECTOR m_rotation;			// 回転
	VECTOR m_scale;				// 拡縮

	MATRIX m_positionMatrix;	// 移動行列
	MATRIX m_rotationMatrix;	// 回転行列
	MATRIX m_scaleMatrix;		// 拡縮行列

	MATRIX m_localMatrix;		// ローカル行列
	MATRIX m_worldMatrix;		// ワールド行列

public:
	Transform();
	Transform(VECTOR position, VECTOR rotation, VECTOR scale);
	virtual ~Transform();

private: // 行列更新用
	void UpdatePositionMatrix(); // 移動行列の更新
	void UpdateRotationMatrix(); // 回転行列の更新
	void UpdateScaleMatrix();    // 拡縮行列の更新

public: // 行列更新用
	void UpdateMatrix();		// 行列の更新
	void LocalToWorldMatrix();	// ローカル→ワールド変換

	VECTOR MatrixToVector();		// 行列からベクトル変換

public: // ラジアン・度変換
	/// <summary>
	/// ラジアンを度に変換
	/// </summary>
	/// <param name="num"></param>
	/// <returns></returns>
	float RadToDeg(float& num); // ラジアンを度に変換

	/// <summary>
	/// 度をラジアンに変換
	/// </summary>
	/// <param name="num"></param>
	/// <returns></returns>
	float DegToRad(float& num); // 度をラジアンに変換

public:// 親子関係設定
	void SetParent(std::weak_ptr<Transform> parent); // 親Transformの設定
	void ClearParent();                             // 親Transformのクリア

public: // ゲッター・セッター
	VECTOR GetPosition() const { return m_position; }											// 位置の取得
	void SetPosition(const VECTOR& position) { m_position = position; }							// 位置の設定
	VECTOR GetRotation() const { return m_rotation; }											// 回転の取得
	void SetRotation(const VECTOR& rotation) { m_rotation = rotation; }							// 回転の設定
	VECTOR GetScale() const { return m_scale; }													// 拡縮の取得
	void SetScale(const VECTOR& scale) { m_scale = scale; }										// 拡縮の設定
	MATRIX GetPositionMatrix() const { return m_positionMatrix; }								// 移動行列の取得
	void SetPositionMatrix(const MATRIX& positionMatrix) { m_positionMatrix = positionMatrix; } // 移動行列の設定
	MATRIX GetRotationMatrix() const { return m_rotationMatrix; }								// 回転行列の取得
	void SetRotationMatrix(const MATRIX& rotationMatrix) { m_rotationMatrix = rotationMatrix; } // 回転行列の設定
	MATRIX GetScaleMatrix() const { return m_scaleMatrix; }										// 拡縮行列の取得
	void SetScaleMatrix(const MATRIX& scaleMatrix) { m_scaleMatrix = scaleMatrix; } 			// 拡縮行列の設定
	void SetLocalMatrix(const MATRIX& localMatrix) { m_localMatrix = localMatrix; }				// ローカル行列の設定
	MATRIX GetLocalMatrix() const { return m_localMatrix; }										// ローカル行列の取得
	void SetWorldMatrix(const MATRIX& worldMatrix) { m_worldMatrix = worldMatrix; }				// ワールド行列の設定
	MATRIX GetWorldMatrix() const { return m_worldMatrix; }										// ワールド行列の取得

	std::shared_ptr<Transform> GetParentTransform() const { auto parent = m_parent.lock(); return parent ? parent : nullptr; } // 親Transformの取得
	// 親Transformが存在しない場合は nullptr を返す
};