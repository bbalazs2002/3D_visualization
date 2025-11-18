#version 430

layout(std430, binding = 0) buffer Positions {
	vec4 pos[];
};

// Variables going forward through the pipeline
out vec3 vs_out_color;

// External parameters of the shader
uniform mat4 viewProj;
uniform int SSBOPadding = 5;

void main()
{
	uint index = gl_VertexID + SSBOPadding;
	mat4 invM = mat4(pos[1], pos[2], pos[3], pos[4]);	// texture to world transformation

	if (gl_VertexID == 0 || gl_VertexID == 1) {
		gl_Position = viewProj * pos[index];
	}
	else
	{
		gl_Position = viewProj * invM * pos[index];
	}
	vs_out_color = vec3(1. - (float(gl_VertexID) / float(pos[0].x) * .5), 0 ,0);
}

