#ifndef __MATERIAL_H__
#define	__MATERIAL_H__

#include <string>
#include <tiny_obj_loader.h>

#include "Texture.h"

class Material
{
public:
	Material();
	virtual ~Material();

	void InitWithTinyMat(tinyobj::material_t* mat, std::string& basePath);
	inline VkDescriptorSet* GetDescriptorSets() { return desc_sets; }

	void PrepareToDraw() { desc_sets_updated = false; }
	bool IsDescSetUpdated() { return desc_sets_updated; }
	void SetDescUpdated() { desc_sets_updated = true; }

	inline Texture* GetAmbientTexture() { return ambient_tex; }
	inline Texture* GetDiffuseTexture() { return diffuse_tex; }

private:
	tinyobj::material_t* tiny_mat;

	Texture* ambient_tex;
	Texture* diffuse_tex;
	Texture* specular_tex;
	Texture* specular_highlight_tex;
	Texture* bump_tex;
	Texture* displacement_tex;
	Texture* alpha_tex;
	Texture* reflection_tex;

	bool desc_sets_updated;
	VkDescriptorSet desc_sets[2];
};

#endif // !__MATERIAL_H__
