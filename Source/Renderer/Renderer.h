#ifndef __RENDERER_H__
#define __RENDERER_H__

class Renderer
{
public:
	Renderer() {}
	virtual ~Renderer() {}

	virtual void Flush() {}
};

#endif // !__RENDERER_H__
