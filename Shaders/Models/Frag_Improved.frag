#version 430

#define FRAGMENT_SHADER

//
// VARIABLES IN THE PIPELINE
//
in vec2 vs_out_tex;
in vec3 vs_out_norm;

out vec4 fs_out_col;

//
// UNIFORMS
//

// textures
uniform sampler2D texImage;

//
// MAIN FUNCTION
//
void main() {

    fs_out_col = vec4(1,0,0,1);
}