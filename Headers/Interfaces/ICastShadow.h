#pragma once

#include "../include_all.h"

interface ICastShadow{
	virtual void RenderShadowMap(GLint lightID, GLint faceID = 0) = 0;
	virtual void SetProgramShadowID(GLuint id) = 0;
	virtual GLuint GetProgramShadowID() const = 0;
	virtual void SetCastShadow(bool cast) = 0;
	virtual bool GetCastShadow() const = 0;
};