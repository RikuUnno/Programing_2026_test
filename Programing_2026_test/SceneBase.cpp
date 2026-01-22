#include "SceneBase.h"

std::weak_ptr<SceneBase> SceneBase::s_currentScene;

void SceneBase::SetCurrentScene(const std::shared_ptr<SceneBase>& scene)
{
 s_currentScene = scene;
}

std::weak_ptr<SceneBase> SceneBase::GetCurrentSceneWeak()
{
 return s_currentScene;
}
