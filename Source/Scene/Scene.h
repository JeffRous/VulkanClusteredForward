#ifndef __SCENE_H__
#define	__SCENE_H__

class Scene
{
public:
	Scene();
	virtual ~Scene();
	
	virtual bool OnEnter() { return false; }
	virtual bool OnUpdate(float dt) { return false; }
	virtual void OnExit() {}
};

#endif // !__SCENE_H__
