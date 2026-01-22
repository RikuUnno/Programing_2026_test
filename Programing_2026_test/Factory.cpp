#include "Factory.h"

// 非テンプレート部分（シングルトンのインスタンス提供）
Factory& Factory::GetInstance()
{
	static Factory instance;
	return instance;
}