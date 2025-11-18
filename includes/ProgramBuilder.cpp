#include "ProgramBuilder.h"
#include "GLUtils.hpp"
#include <sstream>
#include <SDL2/SDL_log.h>
#include <iostream>
#include "../Headers/Log.h"

ProgramBuilder::ProgramBuilder(const GLuint _programID) : programID(_programID)
{
	if (programID == 0)
	{
		SDL_LogMessage(SDL_LOG_CATEGORY_ERROR,
						SDL_LOG_PRIORITY_ERROR,
						"Program needs to be inited before loading!");
		return;
	}

	Log::logToConsole("Program created (ID: ", programID, ")");
}

ProgramBuilder::~ProgramBuilder()
{

}

ProgramBuilder& ProgramBuilder::ShaderStage( const GLenum shaderType, const std::filesystem::path& filename)
{
	Log::logToConsole("Attaching shader ", filename);

    AttachShader( programID, shaderType, filename );
    return *this;
}

void ProgramBuilder::Link()
{
    LinkProgram( programID, true );
}