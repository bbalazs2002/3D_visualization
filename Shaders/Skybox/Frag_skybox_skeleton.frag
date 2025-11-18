#version 430

// pipeline-ból bejövő per-fragment attribútumok
in vec3 vs_out_pos;

out vec4 fs_out_col;

// skybox textúra
uniform samplerCube skyboxTexture;

/*
uniform vec3 color1 = vec3(1.0, 1.0, 0.0);
uniform vec3 color2 = vec3(1.0, 0.0, 1.0);
uniform vec3 color3 = vec3(0.0, 1.0, 1.0);
uniform vec3 color4 = vec3(1.0, 0.0, 1.0);
*/

void main()
{
	// fs_out_col = vec4( 1.0,0.0,1.0,1.0);

	/*
	vec3 viewDir = normalize(vs_out_pos);
	vec3 color;

	if (viewDir.y < 0.0) {		// y in [-1, 0)
		color = mix(color1, color2, -viewDir.y);
	} else {					// y in [0, 1]
		color = mix(color3, color4, viewDir.y);
	}
	fs_out_col = vec4( color,1.0);
	*/

	fs_out_col = texture(skyboxTexture, vs_out_pos);

}