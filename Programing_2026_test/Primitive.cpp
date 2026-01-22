#include "Primitive.h"

// コンストラクタ
Primitive::Primitive(const std::weak_ptr<SceneBase> scene)
	: GameObject(scene)
{}

// デストラクタ
Primitive::~Primitive()
{}

// Draw Update Start Endは継承先で実装