#include "SampleScene.h"
#include "Application/Application.h"
#include "Renderer/VRenderer.h"

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
	colorR = 0.0f;
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
	vkCmdDraw(cb, 3, 1, 0, 0);
}

void SampleScene::OnExit()
{
	
}