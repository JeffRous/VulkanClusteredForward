#include "Material.h"

Material::Material()
{

}

Material::~Material()
{
	if (ambient_tex != NULL)
		delete ambient_tex;
	if (diffuse_tex != NULL)
		delete diffuse_tex;
	if (specular_tex != NULL)
		delete specular_tex;
	if (specular_highlight_tex != NULL)
		delete specular_highlight_tex;
	if (bump_tex != NULL)
		delete bump_tex;
	if (displacement_tex != NULL)
		delete displacement_tex;
	if (alpha_tex != NULL)
		delete alpha_tex;
	if (reflection_tex != NULL)
		delete reflection_tex;
}

void Material::InitWithTinyMat(tinyobj::material_t* mat)
{
	tiny_mat = mat;

	/// load textures
	if(mat->ambient_texname != "")
		ambient_tex = new Texture(mat->ambient_texname);
	if (mat->diffuse_texname != "")
		diffuse_tex = new Texture(mat->diffuse_texname);
	if (mat->specular_texname != "")
		specular_tex = new Texture(mat->specular_texname);
	if (mat->specular_highlight_texname != "")
		specular_highlight_tex = new Texture(mat->specular_highlight_texname);
	if (mat->bump_texname != "")
		bump_tex = new Texture(mat->bump_texname);
	if (mat->displacement_texname != "")
		displacement_tex = new Texture(mat->displacement_texname);
	if (mat->alpha_texname != "")
		alpha_tex = new Texture(mat->alpha_texname);
	if (mat->reflection_texname != "")
		reflection_tex = new Texture(mat->reflection_texname);
}