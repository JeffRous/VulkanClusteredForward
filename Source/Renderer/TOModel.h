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
	virtual void Draw();

	bool LoadTestData();	/// test usage

private:
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	/// renderering data
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
};

#endif // !__TO_MODEL_H__
