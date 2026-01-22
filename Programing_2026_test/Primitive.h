// GameObjectクラスを継承したPrimitiveクラスのヘッダファイル(図形の基底クラス)
#pragma once
#include "GameObject.h"

class Primitive
	: public GameObject
{
protected:
	// 継承でしか使わないのでprotected
	Primitive(const std::weak_ptr<SceneBase> scene);
	virtual ~Primitive();

	// Draw Update Start Endは継承先で実装


};