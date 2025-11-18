#version 430

in vec3 vs_out_pos;

out vec4 vs_out_col;

uniform vec3 selColor = vec3(1, 0, 0);

void main() {
	vs_out_col = vec4(selColor, 0);
}