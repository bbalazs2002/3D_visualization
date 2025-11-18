#pragma once

#include <filesystem>
#include <GL/glew.h>

class ProgramBuilder
{
private:
	const GLuint programID;
protected:
	//void LoadShader(const GLuint, const std::filesystem::path&);
	//void CompileShaderFromSource(const GLuint, std::string_view);
	//void AttachShader(const GLuint, const std::filesystem::path&);
public:
	ProgramBuilder(GLuint);
	~ProgramBuilder();
	ProgramBuilder& ShaderStage(const GLenum, const std::filesystem::path&);
	void Link();
};