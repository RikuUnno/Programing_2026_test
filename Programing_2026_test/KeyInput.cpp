#include "KeyInput.h"
#include "DxLib.h"
#include <algorithm>

KeyInput::KeyInput()
{
    Initialize();
    GetHitKeyStateAll(m_previousKey); // 現在の押下情報の更新
}

KeyInput::~KeyInput()
{}

void KeyInput::Initialize()
{
    IsKeyInputON = false;
    std::fill(std::begin(m_currntKey), std::end(m_currntKey), NULL);
    std::fill(std::begin(m_previousKey), std::end(m_previousKey), NULL);
    std::fill(std::begin(m_repeatedTime), std::end(m_repeatedTime), 1.0);
    std::fill(std::begin(m_repeatedTimer), std::end(m_repeatedTimer), 0.0);
}

bool KeyInput::IsKeyInputTrigger(int KeyCode)
{
	if (!IsKeyInputON) return false;

	bool flag = false;

    GetHitKeyStateAll(m_currntKey); // 現在の押下情報の更新

	((m_currntKey[KeyCode] != m_previousKey[KeyCode]) && (m_currntKey[KeyCode] != NULL)) ? flag = true : flag = false;

	m_previousKey[KeyCode] = m_currntKey[KeyCode]; // 次フレームの為に入れ替え

#ifdef _DEBUG
    // デバッグ表示
    DrawFormatString(0, 90, 0xFFFFFF, "RepeatFlag: %d", flag);
#endif // _DEBUG

	return flag;
}

bool KeyInput::IsKeyInputHeld(int KeyCode)
{
	if (!IsKeyInputON) return false;

	return CheckHitKey(KeyCode); // 連続で返す
}

bool KeyInput::IsKeyInputReleased(int KeyCode)
{
	if (!IsKeyInputON) return false;

	bool flag = false;

    GetHitKeyStateAll(m_currntKey); // 現在の押下情報の更新

	((m_previousKey[KeyCode] == 1) && (m_currntKey[KeyCode] == NULL)) ? flag = true : flag = false;

	m_previousKey[KeyCode] = m_currntKey[KeyCode]; // 次フレームの為に入れ替え

#ifdef _DEBUG
    // デバッグ表示
    DrawFormatString(0, 75, 0xFFFFFF, "Flag: %d", flag);
#endif // _DEBUG

	return flag;
}

bool KeyInput::IsKeyInputRepeated(int KeyCode)
{
    if (!IsKeyInputON) return false;

    bool flag = false;

    GetHitKeyStateAll(m_currntKey);

    // キーが押されている場合
    if (m_currntKey[KeyCode] == 1)
    {
        m_repeatedTimer[KeyCode] += m_repeatedTime[KeyCode] * Time::GetInstance().DeltaTime();

        // 一定時間を超えたらリピート判定
        if (m_repeatedTimer[KeyCode] >= m_repeatedTime[KeyCode])
        {
            flag = true;
            m_repeatedTimer[KeyCode] = 0.0f; // タイマーリセットして次
        }
    }
    else
    {
        // キーが離されたらリセット
        m_repeatedTimer[KeyCode] = m_repeatedTime[KeyCode];
    }

#ifdef _DEBUG
    // デバッグ表示
    DrawFormatString(0, 75, 0xFFFFFF, "DeltaTime: %f", Time::GetInstance().DeltaTime());
    DrawFormatString(0, 90, 0xFFFFFF, "m_repeatedTimer: %f", m_repeatedTimer);
    DrawFormatString(0, 105, 0xFFFFFF, "Flag: %d", flag);
#endif // _DEBUG

    return flag;
}

void KeyInput::SetInputRepeatedTime(int KeyCode, double SetTime)
{
	m_repeatedTime[KeyCode] = SetTime; // 引数内の数値に変更	
}

void KeyInput::BeginKeyInput()
{
    IsKeyInputON = true;
}

void KeyInput::EndKeyInput()
{
    IsKeyInputON = false;
}