#include "SampleScene.h"
#include "Application/Application.h"
#include "Renderer/VRenderer.h"
#include "Renderer/TOModel.h"

SampleScene::SampleScene()
{
	camera = new Camera(Application::Inst()->GetWidth(), Application::Inst()->GetHeight());
	Application::Inst()->SetRendererCamera(camera);
	model = NULL;
}

SampleScene::~SampleScene()
{
	if( camera != NULL )
		delete camera;
}

bool SampleScene::OnEnter()
{
	colorR = 0.0f;
	model = new TOModel();
	model->LoadTestData();
	return true;
}

bool SampleScene::OnUpdate(float dt)
{
	colorR += 0.2f * dt;
	if (colorR >= 1.0f)
		colorR = 0.0f;
	return true;
}

void SampleScene::OnRender(Renderer* render)
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)render;
	VkCommandBuffer cb = vRenderer->CurrentCommandBuffer();
	vRenderer->SetClearColor({ colorR ,0,0,1});
	
	model->Draw();
}

void SampleScene::OnExit()
{
	if (model != NULL)
	{
		delete model;
	}
}