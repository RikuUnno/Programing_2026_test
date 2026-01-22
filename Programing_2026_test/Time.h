#pragma once

class Time
{
private:
	double m_currentFrame; // 現在のフレーム
	double m_previousFrame; // 過去のフレーム
	double m_deltaTime; // フレーム間の差分

private:
	// コンストラクタ・コピー禁止
	Time();                          // 外部からnewできない
	Time(const Time&) = delete;      // コピー禁止
	Time& operator=(const Time&) = delete; // 代入禁止

public:
	virtual ~Time(); // デストラクタ

public:
	// インスタンス
	static Time& GetInstance();

public:
	void Update(); // 更新

public:	// 機能
	double DeltaTime(); // デルタタイムの取得
};