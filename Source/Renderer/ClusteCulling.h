#ifndef __CLUSTE_CULLING_H__
#define	__CLUSTE_CULLING_H__

namespace RawCpu
{
	static glm::vec3 lineIntersectionToZPlane(glm::vec3& A, glm::vec3& B, float zDistance) {
		//Because this is a Z based normal this is fixed
		glm::vec3 normal = { 0.0, 0.0, 1.0 };

		glm::vec3 ab = B - A;

		//Computing the intersection length for the line and the plane
		float t = (zDistance - glm::dot(normal, A)) / glm::dot(normal, ab);

		//Computing the actual xyz position of the point along the line
		glm::vec3 result = A + t * ab;

		return result;
	}

	static glm::vec4 clipToView(glm::vec4& clip, ScreenToView& screenToView) {
		//View space transform
		glm::vec4 view = screenToView.inverseProjection * clip;

		//Perspective projection
		view = view / view.w;

		return view;
	}

	static glm::vec4 screen2View(glm::vec4& screen, ScreenToView& screenToView)
	{
		//Convert to NDC
		glm::vec2 texCoord = glm::vec2(screen.x / screenToView.screenDimensions.x, screen.y / screenToView.screenDimensions.y);

		//Convert to clipSpace
		// vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
		glm::vec4 clip = glm::vec4(glm::vec2(texCoord.x, texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);
		//Not sure which of the two it is just yet

		return clipToView(clip, screenToView);
	}

	static float sqDistPointAABB(glm::vec3& point, glm::vec3& minPoint, glm::vec3& maxPoint)
	{
		float sqDist = 0.0;
		for (int i = 0; i < 3; ++i) {
			float v = point[i];
			if (v < maxPoint[i]) {
				sqDist += (maxPoint[i] - v) * (maxPoint[i] - v);
			}
			if (v > maxPoint[i]) {
				sqDist += (v - maxPoint[i]) * (v - maxPoint[i]);
			}
		}

		return sqDist;
	}

	static bool testSphereAABB(ScreenToView& screenToView, glm::vec3& pos, float radius, glm::vec3& minPoint, glm::vec3& maxPoint)
	{
		glm::vec3 center = glm::vec3(screenToView.viewMatrix*glm::vec4(pos, 1.0f));
		float squaredDistance = sqDistPointAABB(center, minPoint, maxPoint);

		bool ret = (squaredDistance <= (radius * radius));
		return ret;
	}

	static void cluste_culling(int xSize, int ySize, int zSize, ScreenToView& screenToView, PointLightData* pointLights, int lightCount, LightGrid* lightGrids, glm::uint* globalLightIndexList)
	{
		int globalIndexCount = 0;

		//Eye position is zero in view space
		glm::vec3 eyePos = glm::vec3(0.0);
		float zNear = screenToView.zNear;
		float zFar = screenToView.zFar;

		//Per Tile variables
		float tileSizePx = screenToView.tileSizes[3];
		for (int x = 0; x < xSize; x++)
		{
			for (int y = 0; y < ySize; y++)
			{
				for (int z = 0; z < zSize; z++)
				{
					glm::uint tileIndex = x + y * xSize + z * xSize * ySize;

					//Calculating the min and max point in screen space
					glm::vec4 maxPoint_sS = glm::vec4(glm::vec2(x + 1, y + 1) * tileSizePx, -1.0, 1.0); // Top Right
					glm::vec4 minPoint_sS = glm::vec4(glm::vec2(x, y) * tileSizePx, -1.0, 1.0); // Bottom left

					//Pass min and max to view space
					glm::vec3 maxPoint_vS = glm::vec3(screen2View(maxPoint_sS, screenToView));
					glm::vec3 minPoint_vS = glm::vec3(screen2View(minPoint_sS, screenToView));

					//Near and far values of the cluster in view space
					float tileNear = -zNear * pow(zFar / zNear, (float)z / zSize);
					float tileFar = -zNear * pow(zFar / zNear, (float)(z + 1) / zSize);

					//Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
					glm::vec3 minPointNear = lineIntersectionToZPlane(eyePos, minPoint_vS, tileNear);
					glm::vec3 minPointFar = lineIntersectionToZPlane(eyePos, minPoint_vS, tileFar);
					glm::vec3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileNear);
					glm::vec3 maxPointFar = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileFar);

					glm::vec3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
					glm::vec3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

					glm::uint visibleLightIndices[100];
					glm::uint visibleLightCount = 0;

					for (int light = 0; light < lightCount; light++)
					{
						if (pointLights[light].enabled == 1)
						{
							if (testSphereAABB(screenToView, pointLights[light].pos, pointLights[light].radius, minPointAABB, maxPointAABB))
							{
								visibleLightIndices[visibleLightCount] = light;
								visibleLightCount += 1;
							}
						}
					}

					///glm::uint offset = atomic_add_global(&globalIndexCount, visibleLightCount);
					glm::uint offset = globalIndexCount;
					globalIndexCount += visibleLightCount;

					for (glm::uint i = 0; i < visibleLightCount; ++i)
					{
						globalLightIndexList[offset + i] = visibleLightIndices[i];
					}

					lightGrids[tileIndex].offset = offset;
					lightGrids[tileIndex].count = visibleLightCount;
				}
			}
		}
	}
}

#endif // !__CLUSTE_CULLING_H__

