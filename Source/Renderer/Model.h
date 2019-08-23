#ifndef __MODEL_H__
#define __MODEL_H__

#include <iostream>

#include "TransformEntity.h"

class Model : public TransformEntity
{
public:
	Model() { transform_changed = true; }
	virtual ~Model() {}

	virtual bool LoadFromPath(std::string path) = 0;
	virtual void Draw() = 0;
};

#endif // !__MODEL_H__