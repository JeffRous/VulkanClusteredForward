#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Renderer/VRenderer.h"
#include "Camera.h"
#include "Application/Application.h"
#include "TOModel.h"

const std::vector<Vertex> vertices = {
	{{0.0f, -2.5f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	{{2.5f, 2.5f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	{{-2.5f, 2.5f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

	{{0.0f, 0.0f, 0.01f, 1.0f}, {1.0f, 0.0f, 0.0f}},
	{{5.0f, -2.5f, 0.01f, 1.0f}, {0.0f, 1.0f, 0.0f}},
	{{-2.5f, -2.5f, 0.01f, 1.0f}, {0.0f, 0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 3, 5, 4
};

TOModel::TOModel()
{
	
}

TOModel::~TOModel()
{
	for (int i = 0; i < material_insts.size(); i++)
	{
		delete material_insts[i];
	}
	material_insts.clear();

	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();

	for (int i = 0; i < index_buffers.size(); i++)
	{
		vRenderer->CleanBuffer(index_buffers[i], index_buffer_memorys[i]);
	}
	index_buffers.clear();
	index_buffer_memorys.clear();
	for (int i = 0; i < vertex_buffers.size(); i++)
	{
		vRenderer->CleanBuffer(vertex_buffers[i], vertex_buffer_memorys[i]);
	}
	vertex_buffers.clear();
	vertex_buffer_memorys.clear();
}

bool TOModel::LoadTestData()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	vRenderer->CreateVertexBuffer((void*)vertices.data(), sizeof(vertices[0]), vertices.size(), vertex_buffer, vertex_buffer_memory);
	vertex_buffers.push_back(vertex_buffer);
	vertex_buffer_memorys.push_back(vertex_buffer_memory);
	vRenderer->CreateIndexBuffer((void*)indices.data(), sizeof(indices[0]), indices.size(), index_buffer, index_buffer_memory);
	index_buffers.push_back(index_buffer);
	index_buffer_memorys.push_back(index_buffer_memory);
	indices_counts.push_back(static_cast<uint32_t>(indices.size()));

	return true;
}

void TOModel::Draw()
{
	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	VkCommandBuffer cb = vRenderer->CurrentCommandBuffer();
	
	/// Set model view matrix
	glm::mat4x4* modelViewMatrix = UpdateMatrix();
	Camera* cam = vRenderer->GetCamera();
	if (cam != NULL)
	{	/// set mvp to shader
		glm::mat4x4 mvp = (*cam->GetViewProjectMatrix())*(*modelViewMatrix);
		///glm::vec4 test_p = mvp * glm::vec4(0.0f, -2.5f, -4.9f, 1.0f);	/// vtx output z is 0-1 for(-4.9 and 95)
		vRenderer->SetMvpMatrix(mvp);
		vRenderer->SetModelMatrix(*modelViewMatrix);
		vRenderer->SetViewMatrix(*cam->GetViewMatrix());
		vRenderer->SetProjMatrix(*cam->GetProjectMatrix());
		vRenderer->SetProjViewMatrix(*cam->GetViewProjectMatrix());
		vRenderer->SetCamPos(cam->GetPosition());
	}

	/// prepare materials
	for (int i = 0; i < material_insts.size(); i++)
	{
		material_insts[i]->PrepareToDraw();
	}

	/// use index buffer
	if (index_buffers.size() > 0)
	{
		for (int i = 0; i < vertex_buffers.size(); i++)
		{
			VkBuffer vertexBuffers[] = { vertex_buffers[i] };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cb, i, 1, vertexBuffers, offsets);
		}
		for (int i = 0; i < index_buffers.size(); i++)
		{
			if (mat_ids.size() > i && mat_ids[i] >= 0 && material_insts.size() > mat_ids[i] && material_insts[mat_ids[i]] != NULL)
			{
				/// material
				Material* mat = material_insts[mat_ids[i]];
				vRenderer->SetTexture(mat->GetDiffuseTexture());
				vRenderer->SetNormalTexture(mat->GetNormalTexture());
				vRenderer->UpdateMaterial(mat);
			}
			vkCmdBindIndexBuffer(cb, index_buffers[i], 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(cb, indices_counts[i], 1, 0, 0, 0);
		}
	}
	else
	{
		for (int i = 0; i < vertex_buffers.size(); i++)
		{
			if (mat_ids.size() > i && mat_ids[i] >= 0 && material_insts.size() > mat_ids[i] && material_insts[mat_ids[i]] != NULL)
			{
				/// material
				Material* mat = material_insts[mat_ids[i]];
				vRenderer->SetTexture(mat->GetDiffuseTexture());
				vRenderer->SetNormalTexture(mat->GetNormalTexture());
				vRenderer->UpdateMaterial(mat);
			}
			VkBuffer vertexBuffers[] = { vertex_buffers[i] };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(cb, 0, 1, vertexBuffers, offsets);
			vkCmdDraw(cb, indices_counts[i], 1, 0, 0);
		}
	}
}

bool TOModel::LoadFromPath(std::string path)
{
	std::string err;
	std::string warn;

	///std::string filename = "Data/sponza_full/sponza.obj";
	std::string basePath = "";
	int slashIdx = path.find_last_of("/");
	if (slashIdx >= 0)
	{
		basePath = path.substr(0, slashIdx);
	}

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), basePath.c_str()))
	{
		throw std::runtime_error(err);
		return false;
	}

	/// material instances
	for (int i = 0; i < materials.size(); i++)
	{
		Material* mat = new Material();
		mat->InitWithTinyMat(&materials[i], basePath);
		material_insts.push_back(mat);
	}

	VulkanRenderer* vRenderer = (VulkanRenderer*)Application::Inst()->GetRenderer();
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;

	/// multi vertex buffers
	bool hasWeight = attrib.vertex_weights.size() > 0;
	bool hasWs = attrib.texcoord_ws.size() > 0;
	for (int i = 0; i < shapes.size(); i++)
	{
		tinyobj::shape_t* shape = &shapes[i];
		tinyobj::mesh_t* mesh = &shape->mesh;
		int totalVtxNum = mesh->indices.size();
		assert(mesh->num_face_vertices.size() == totalVtxNum / 3);

		/// sub mesh precalculate
		std::vector<int> subMeshTriIdxs;
		std::vector<int> subMeshMatIds;
		int matId = mesh->material_ids[0];
		for (int j = 0; j < mesh->material_ids.size(); j++)
		{
			if (mesh->material_ids[j] != matId)
			{
				subMeshMatIds.push_back(matId);
				subMeshTriIdxs.push_back(j);
				matId = mesh->material_ids[j];
			}
		}
		subMeshMatIds.push_back(matId);
		subMeshTriIdxs.push_back((int)mesh->material_ids.size());

		/// sub mesh vb
		int startVtxIdx = 0;
		for (int k = 0; k < subMeshMatIds.size(); k++)
		{
			int vtxNum = (subMeshTriIdxs[k] - startVtxIdx) * 3;
			Vertex* vertices = new Vertex[vtxNum];
			for (int j = 0; j < vtxNum; j++)
			{
				int idx = mesh->indices[startVtxIdx * 3 + j].vertex_index;
				vertices[j].pos.x = attrib.vertices[idx * 3 + 0];
				vertices[j].pos.y = attrib.vertices[idx * 3 + 1];
				vertices[j].pos.z = attrib.vertices[idx * 3 + 2];
				vertices[j].pos.w = 1.0f;
				if (hasWeight)
					vertices[j].pos.w = attrib.vertex_weights[idx];
				vertices[j].color.r = attrib.colors[idx * 3 + 0];
				vertices[j].color.g = attrib.colors[idx * 3 + 1];
				vertices[j].color.b = attrib.colors[idx * 3 + 2];

				idx = mesh->indices[startVtxIdx * 3 + j].texcoord_index;
				vertices[j].texcoord.x = attrib.texcoords[idx * 2 + 0];
				vertices[j].texcoord.y = 1.0f - attrib.texcoords[idx * 2 + 1];
				if (hasWs)
				{
					vertices[j].texcoord.z = attrib.texcoord_ws[idx];
				}

				idx = mesh->indices[startVtxIdx * 3 + j].normal_index;
				vertices[j].normal.x = attrib.normals[idx * 3 + 0];
				vertices[j].normal.y = attrib.normals[idx * 3 + 1];
				vertices[j].normal.z = attrib.normals[idx * 3 + 2];
				vertices[j].normal.w = 1.0f;

				/// calculate tangent/bitangent
				if ((j % 3) == 2)
				{
					glm::vec4 & v0 = vertices[j - 2].pos;
					glm::vec4 & v1 = vertices[j - 1].pos;
					glm::vec4 & v2 = vertices[j].pos;

					// Shortcuts for UVs
					glm::vec3 & uv0 = vertices[j - 2].texcoord;
					glm::vec3 & uv1 = vertices[j - 1].texcoord;
					glm::vec3 & uv2 = vertices[j].texcoord;

					// Edges of the triangle : position delta
					glm::vec3 deltaPos1 = v1 - v0;
					glm::vec3 deltaPos2 = v2 - v0;

					// UV delta
					glm::vec2 deltaUV1 = uv1 - uv0;
					glm::vec2 deltaUV2 = uv2 - uv0;

					float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
					glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
					glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

					vertices[j - 2].tangent = tangent;
					vertices[j - 1].tangent = tangent;
					vertices[j].tangent = tangent;

					vertices[j - 2].bitangent = bitangent;
					vertices[j - 1].bitangent = bitangent;
					vertices[j].bitangent = bitangent;
				}
			}
			vRenderer->CreateVertexBuffer((void*)vertices, sizeof(Vertex), vtxNum, vertex_buffer, vertex_buffer_memory);
			vertex_buffers.push_back(vertex_buffer);
			vertex_buffer_memorys.push_back(vertex_buffer_memory);
			indices_counts.push_back(static_cast<uint32_t>(vtxNum));
			mat_ids.push_back(subMeshMatIds[k]);
			delete[] vertices;

			startVtxIdx = subMeshTriIdxs[k];
		}
	}

	return true;
}