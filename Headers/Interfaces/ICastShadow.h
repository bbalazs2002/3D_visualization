#pragma once

#include "../include_all.h"

interface ICastShadow{
	virtual void RenderShadowMap(RenderParams* p) = 0;
};