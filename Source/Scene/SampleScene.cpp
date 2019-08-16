#include "SampleScene.h"
#include "Application/Application.h"

SampleScene::SampleScene()
{
	camera = new Camera(Application::Inst()->GetWidth(), Application::Inst()->GetHeight());
	Application::Inst()->SetRendererCamera(camera);
}

SampleScene::~SampleScene()
{
	if( camera != NULL )
		delete camera;
}

bool SampleScene::OnEnter()
{
	return true;
}

bool SampleScene::OnUpdate(float dt)
{
	return true;
}

void SampleScene::OnExit()
{
	
}