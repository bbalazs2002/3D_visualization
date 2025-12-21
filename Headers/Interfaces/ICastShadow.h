#pragma once

#include "../include_all.h"

interface ICastShadow{
	virtual void RenderShadowMap(RenderParams* p) = 0;
	virtual void SetProgramShadowID(GLuint id) = 0;
	virtual GLuint GetProgramShadowID() const = 0;
	virtual void SetShadowCasting(bool cast) = 0;
	virtual bool GetShadowCasting() const = 0;
};