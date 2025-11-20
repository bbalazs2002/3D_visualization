#pragma once

#include "../include_all.h"

interface IDrawable{
	virtual void Render(RenderParams* p) = 0;
	virtual void RenderSelection(RenderParams* p) = 0;
	// virtual void RenderShadow(RenderParams* p, Light* l) = 0;

	virtual void RenderGUIBase() = 0;
	virtual void RenderGUI(std::vector<ModelBase*>* models) = 0;
};