#ifndef __MODEL_H__
#define __MODEL_H__

#include <iostream>

class Model
{
public:
	Model() {}
	virtual ~Model() {}

	virtual bool LoadFromPath(std::string path) { return false; }
};

#endif // !__MODEL_H__