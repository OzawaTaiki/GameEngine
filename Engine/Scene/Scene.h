#pragma once

class Scene
{
public:

	virtual ~Scene() {};

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	bool isChange_ = false;
};