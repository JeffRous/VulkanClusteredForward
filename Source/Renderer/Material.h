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

	void InitWithTinyMat(tinyobj::material_t* mat);

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
};

#endif // !__MATERIAL_H__
