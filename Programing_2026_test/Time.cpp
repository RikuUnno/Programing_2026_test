#include "Time.h"
#include "DxLib.h"

// コンストラクタ
Time::Time()
{
	m_currentFrame = 0;
	m_previousFrame = 0;
	m_deltaTime = 0;
}

// デストラクタ
Time::~Time()
{

}

Time& Time::GetInstance()
{
	static Time instance;
	return instance;
}

// 更新
void Time::Update()
{
	double now = GetNowCount();

	if (now < m_currentFrame) now += 0xFFFFFFFF;

	// deltaTimeの処理
	m_previousFrame = m_currentFrame; // 前フレームの時間の保存
	m_currentFrame = now;	// 現在のフレームの経過時間
	m_deltaTime = (m_currentFrame - m_previousFrame) / 1000.0; // ミリ秒→秒

}

// デルタタイムの取得
double Time::DeltaTime()
{
	return m_deltaTime;
}