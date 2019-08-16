#ifndef __TO_MODEL_H__
#define __TO_MODEL_H__

#include <tiny_obj_loader.h>
/*view https://github.com/syoyo/tinyobjloader for more informations*/

#include "Model.h"

class TOModel : public Model
{
public:
	TOModel();
	virtual ~TOModel();

	virtual bool LoadFromPath(std::string path);

private:
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
};

#endif // !__TO_MODEL_H__
