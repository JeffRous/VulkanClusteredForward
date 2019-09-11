#include "Application/Application.h"
#include "Renderer/VRenderer.h"
#include "Renderer/TOModel.h"
#include "Renderer/Camera.h"
#include "SampleScene.h"

SampleScene::SampleScene()
{
	camera = new Camera(Application::Inst()->GetWidth(), Application::Inst()->GetHeight());
	Application::Inst()->SetRendererCamera(camera);
	model = NULL;
	last_control_state = 0;
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
	model->LoadFromPath("Data/sponza_full/sponza.obj");
	///glm::vec3 rotate = glm::vec3(-90, 0, 0);
	///model->SetRotation(rotate);
	//model->LoadTestData();
	return true;
}

bool SampleScene::OnUpdate(float dt)
{
	colorR += 0.2f * dt;
	if (colorR >= 1.0f)
		colorR = 0.0f;

	UpdateCameraByInput();

	return true;
}

void SampleScene::UpdateCameraByInput()
{
	int controlState = Application::Inst()->GetControlState();
	if (last_control_state != controlState)
	{
		camera->UpdateLookAt();
		last_control_state = controlState;
	}
	if (controlState == 0)
	{
		float scrollOffset = (Application::Inst()->GetScrollOffset() - camera->GetBaseScrollOffset()) * -10.0f + camera->GetLookAtDist();
		glm::vec3 lookAtDir = camera->GetLookAtDir();
		lookAtDir *= scrollOffset;
		glm::vec3 newPos = camera->GetLookAtPosition() - lookAtDir;
		camera->SetPosition(newPos);
	}
	else if (controlState == 1)	/// rotate
	{
		glm::vec2 rotateOffset = Application::Inst()->GetMoveOffset() - camera->GetBaseMoveOffset();
		glm::vec3 currentLookAtDir = camera->GetCurrentLookAtDir();
		glm::vec3 rotAxis = glm::cross(currentLookAtDir, glm::vec3(0, 1, 0));
		rotAxis = glm::normalize(rotAxis);
		glm::mat rot = glm::identity<glm::mat4x4>();
		rot = glm::rotate(rot, glm::radians(-rotateOffset.y*0.5f), rotAxis);
		rot = glm::rotate(rot, glm::radians(-rotateOffset.x*0.5f), glm::vec3(0, 1, 0));
		glm::vec3 newPos = camera->GetCurrentPosition();
		glm::vec3 lookAt = camera->GetCurrentLookAt();
		glm::vec3 offset = newPos - lookAt;
		newPos = rot * glm::vec4(offset.x, offset.y, offset.z, 1.0f);
		newPos += lookAt;
		camera->SetPosition(newPos);
	}
	else if (controlState == 2)	/// shift
	{
		glm::vec2 shiftOffset = Application::Inst()->GetMoveOffset() - camera->GetBaseMoveOffset();
		glm::mat4x4& mat = camera->GetMatrix();
		glm::vec3 camUp = glm::vec3(mat[0][1], mat[1][1], mat[2][1]);
		glm::vec3 camRight = glm::vec3(mat[0][0], mat[1][0], mat[2][0]);
		glm::vec3 newPos = camera->GetCurrentPosition();
		glm::vec3 newLookat = camera->GetCurrentLookAt();
		glm::vec3 shift = camRight * -shiftOffset.x + camUp * shiftOffset.y;
		newPos += shift;
		newLookat += shift;
		camera->SetPosition(newPos);
		camera->LookAt(newLookat);
	}
}

void SampleScene::OnRender(Renderer* render)
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)render;
	VkCommandBuffer cb = vRenderer->CurrentCommandBuffer();
	//vRenderer->SetClearColor({ colorR ,0,0,1});
	
	model->Draw();
}

void SampleScene::OnExit()
{
	if (model != NULL)
	{
		delete model;
	}
}