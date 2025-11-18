#version 430

layout(std430, binding = 0) buffer Positions {
	vec4 pos[];
};

// Variables going forward through the pipeline
out vec3 color;
out uint vs_out_vertexID;

// External parameters of the shader
uniform mat4 viewProj;
uniform int SSBOPadding = 5;

void main()
{
	vs_out_vertexID = gl_VertexID;
	uint index = gl_VertexID + SSBOPadding + 2;

	gl_Position = vec4(0, 0, 0, 0);
	color = vec3(1. - (float(gl_VertexID) / float(pos[0].x - 3) * .5), 0 ,0);
}

